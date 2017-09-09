#ifndef _LB_NODE_H_
#define _LB_NODE_H_

#include <iostream>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "healthcheck.h"

using namespace std;

class LbNode {
	/* Enums */
	public:
	enum State {
		STATE_DOWN = 0,
		STATE_UP   = 1,
	};

	/* Methods */
	public:
		LbNode(string name, const YAML::Node& config, class LbPool *parent_lbpool, std::string proto, set<string> *downtimes);
		void schedule_healthchecks(struct timespec *now);
		void finalize_healthchecks();
		void node_logic();

		void start_downtime();
		void end_downtime();
		bool is_downtimed();

		State get_state(); /* getter for private member */
		string get_state_text(); /* same, but text representation */

	/* Members */
	public:
		string			 name;
		string			 address; /* libevent wants the address passed as char[] so keep to some string-like. */
		int			 address_family;
		class LbPool		*parent_lbpool;
		State			 state;
		vector<class Healthcheck*> healthchecks;
		bool			 downtime;
		bool			 min_nodes_kept; /* Node was kept to meet min_nodes requirement. */
		bool			 max_nodes_kept; /* Node was kept because it met max_nodes requirement. */
		bool			 checked; /* This node has all of its checks ran at least once. */

	private:
		State			 admin_state;
		bool			 backup;
};

#endif

