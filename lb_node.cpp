#include <iostream>
#include <sstream>

#include "msg.h"
#include "pfctl.h"

#include "lb_pool.h"
#include "lb_node.h"
#include "healthcheck.h"

using namespace std;


/* Global variables. */
extern bool	 	 pf_action;
extern int	 	 verbose;


/*
   The constructor has not much work to do, init some variables
   and display the LbNode address if verbose.
*/
LbNode::LbNode(istringstream &parameters, class LbPool *_parent_lbpool) {
	parent_lbpool = _parent_lbpool;
	parent_lbpool->nodes.push_back(this);

	parameters >> address;

	downtime = false;

	/* Check if the IP address is in pf table. This determines the initial state of node. */
	if (pf_is_in_table(parent_lbpool->name, address)) {
		hard_state  = STATE_UP;
		parent_lbpool->nodes_alive++;
	} else {
		hard_state  = STATE_DOWN;
	}

	if (verbose>0)
		cout << "  New LbNode " << address << " pf_state:" << (hard_state==STATE_DOWN?"DOWN":"UP") << endl;
}


/*
   Try to schedule each healthcheck. Do not try if there is a downtime for this node.
*/
void LbNode::schedule_healthchecks() {
	if (downtime == true)
		return;

	for(unsigned int hc=0; hc<healthchecks.size(); hc++) {
		healthchecks[hc]->schedule_healthcheck();
	}
}


/*
   Check results of all healthchecks for this node and act accordingly:
   - display messages
   - perform pf operations
*/
void LbNode::parse_healthchecks_results() {

	unsigned int all_healthchecks = healthchecks.size();
	unsigned int ok_healthchecks = 0;

	/* Downtime feature - pretend that this node has just failed. */
	if (downtime == true)
		ok_healthchecks = 0;
	else
	/* If there is no downtime, go over all healthchecks for this node and count hard STATE_UP healthchecks. */
	for (unsigned int hc=0; hc<all_healthchecks; hc++) {
		if (healthchecks[hc]->hard_state == STATE_UP)
			ok_healthchecks++;
	}

	/* This node is up and some healthchecks have recently failed. */
	if (hard_state == STATE_UP && ok_healthchecks<all_healthchecks) {

		if (downtime == true)
			showStatus(CL_WHITE"%s"CL_RESET" - "CL_CYAN"%s"CL_RESET" - LbNode:"CL_RED" the node is forced down"CL_RESET"\n",
				parent_lbpool->name.c_str(), address.c_str(), all_healthchecks-ok_healthchecks, all_healthchecks );
		else
			showStatus(CL_WHITE"%s"CL_RESET" - "CL_CYAN"%s"CL_RESET" - LbNode:"CL_RED" the node is down, %d of %d checks failed"CL_RESET"\n",
				parent_lbpool->name.c_str(), address.c_str(), all_healthchecks-ok_healthchecks, all_healthchecks );

		/* Remove the node from its primary pool if the pool is in normal (not backup) operation. */
		if (parent_lbpool->switched_to_backup == false) {
			pf_table_del(parent_lbpool->name, address);
			pf_kill_src_nodes_to(address, true);
		}

		/* Remove the node from other pools when parent pool is used as backup_pool anywhere. */
		if (parent_lbpool->used_as_backup.size()) {
			for (unsigned int bpl=0; bpl<parent_lbpool->used_as_backup.size(); bpl++) {
				if (parent_lbpool->used_as_backup[bpl]->switched_to_backup) {
					showStatus(CL_WHITE"%s"CL_RESET" - Used as backup, removing from another pool: "CL_BLUE"%s"CL_RESET"\n", parent_lbpool->name.c_str(), parent_lbpool->used_as_backup[bpl]->name.c_str());
					pf_table_del(parent_lbpool->used_as_backup[bpl]->name, address);
					pf_kill_src_nodes_to(address, true);
				}
			}
		}

		parent_lbpool->nodes_alive--;

		hard_state = STATE_DOWN;
	}
	/* This node is down and and all healthchecks have recently passed. */
	else if (hard_state == STATE_DOWN && ok_healthchecks==all_healthchecks) {

		showStatus(CL_WHITE"%s"CL_RESET" - "CL_CYAN"%s"CL_RESET" - LbNode:"CL_GREEN" the node is up, %d of %d checks passed"CL_RESET"\n",
			parent_lbpool->name.c_str(), address.c_str(), ok_healthchecks, all_healthchecks) ;

		parent_lbpool->all_down_noticed = false;
		/* Add the node to its primary pool if the pool is in normal (not backup) operation. */
		if (parent_lbpool->switched_to_backup == false) {
			pf_table_add(parent_lbpool->name, address);
			pf_table_rebalance(parent_lbpool->name, address);
		}

		/* Add this node to all pools */
		if (parent_lbpool->used_as_backup.size())
			for (unsigned int bpl=0; bpl<parent_lbpool->used_as_backup.size(); bpl++) {
				if (parent_lbpool->used_as_backup[bpl]->switched_to_backup) {
					showStatus(CL_WHITE"%s"CL_RESET" - Used as backup, adding to other pool: "CL_BLUE"%s"CL_RESET"\n", parent_lbpool->name.c_str(), parent_lbpool->used_as_backup[bpl]->name.c_str());
					pf_table_add(parent_lbpool->used_as_backup[bpl]->name, address);
					/* Do not rebalance traffic in other pools, we don't want to loose src_nodes in them,
					   so killing traffic to backup nodes will be possible later. */
				}
			}

		parent_lbpool->nodes_alive++;

		hard_state = STATE_UP;
	}

}


/*
   Start a downtime.
*/
void LbNode::start_downtime() {
	/* Do not mark the node down twice. */
	if (downtime)
		return;

	showStatus(CL_WHITE"%s"CL_RESET" - Starting downtime for %s...\n", parent_lbpool->name.c_str(), address.c_str());

	downtime = true;
}


/*
   End a downtime.
*/
void LbNode::end_downtime() {
	/* Remove downtime only once. */
	if (!downtime)
		return;

	showStatus(CL_WHITE"%s"CL_RESET" - Ending downtime for %s...\n", parent_lbpool->name.c_str(), address.c_str());

	downtime = false;

	/* Pretend that this host was fully down. */
	hard_state = STATE_DOWN;
	for (unsigned int hc=0; hc<healthchecks.size(); hc++) {
		healthchecks[hc]->downtime_failure();
	}
	
}
