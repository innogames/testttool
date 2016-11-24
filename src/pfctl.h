#ifndef _PFCTL_H_
#define _PFCTL_H_

#include <set>
#include <string>

using namespace std;

void pf_kill_src_nodes_to(string &pool, string &ip, bool with_states);
void pf_kill_states_to_rdr(string &pool, string &redirection, bool with_states);
void pf_table_add(string &table, string &ip);
void pf_table_del(string &table, string &ip);
int  pf_is_in_table(string &table, string &ip);
void pf_table_rebalance(string &table, const set<string> &skip_ips);
const set<string> pf_table_members(string &table);

#endif
