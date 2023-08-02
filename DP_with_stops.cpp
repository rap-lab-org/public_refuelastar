#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <map>
#include <queue>
#include <chrono>

const long INF = std::numeric_limits<long>::max();

struct Edge
{
    long node_from;
    long node_to;
    long refuel_cost;
    long distance;
};

struct Recc
{
    long u;
    long q;
    long g;

    bool operator==(const Recc& other) const {
        return u == other.u && q == other.q && g == other.g;
    }
};

struct ReccHash
{
    std::size_t operator()(const Recc& r) const
    {
        std::size_t h1 = std::hash<long>{}(r.u);
        std::size_t h2 = std::hash<long>{}(r.q);
        std::size_t h3 = std::hash<long>{}(r.g);
        return h1 ^ h2 ^ h3;
    }
};

struct H_Edge
{
    long node_from;
    long gas_at_node_from;
    long node_to;
    long gas_left;
    long cost_trav;
};

struct CompareH_Edge {
    bool operator()(const H_Edge& a, const H_Edge& b) const {
        return a.cost_trav > b.cost_trav; // Use > for min-heap (smallest cost first)
    }
};

class Graph{
private:
    std::vector<Edge> edges;

    long cost(long node) const {
        long cost = 0;
        for (const auto& i : edges) {
            if (node == i.node_from) {
                cost = i.refuel_cost;
                break;
            }
        }
        return cost;
    }

    long Dist(long node_from, long node_to) const {
        long d = 0;
        for (const auto& i : edges) {
            if (i.node_from == node_from && i.node_to == node_to) {
                d = i.distance;
                break;
            }
        }
        return d;
    }

    std::vector<long> Compute_reachable_sets(long node, long qmax) const {
        std::vector<long> R;
        for (const auto& i : edges) {
            if (i.node_from == node && i.distance <= qmax) {
                R.push_back(i.node_to);
            }
        }
        std::sort(R.begin(), R.end());
        return R;
    }    

public:
    Graph() = default;

    void addEdge(const Edge& edge)
    {
        edges.push_back(edge);
    }

    std::unordered_map<long, std::vector<long>> GV(long qmax) const {
        std::unordered_map<long, std::vector<long>> gv;
        for (const auto& i : edges) {
            long curr = i.node_from;
            long to = i.node_to;
            if (cost(to) > cost(curr) && i.distance <= qmax) {
                gv[i.node_to].push_back((qmax - i.distance));
            } else {
                gv[i.node_to].push_back(0);
            }
        }
        for (auto& j : gv) {
            std::sort(j.second.begin(), j.second.end());
        }
        return gv;
    }
    /*
    * The main Fill-Row function
    * */
std::unordered_map<Recc, long, ReccHash>Fill_Row(long u, long q, long qmax,  std::unordered_map<long, std::vector<long>>& gv )
{
    std::unordered_map<Recc, long, ReccHash> A;
    std::vector<long> R = Compute_reachable_sets( u, qmax);
    std::unordered_map<long, long> indepv;
    long curr_node = u;
	long next_node;
	long dest = 3;
    long num_nodes= 3;

    // std::cout<<"-------------------"<< std::endl;
    // std::cout<<"current node :"<< curr_node<< std::endl;
	
    for (long u=0; u<num_nodes; u++)
    {
		for (auto g: gv[u])
		{
			if (g <=Dist(u, dest) && Dist(u, dest) <=qmax)
			{
                // std::cout<<"g :"<< g<< " ";
				A[{u, 1, g}] = (Dist(u, dest) - g)*(cost(u));
				// std::cout<<" q = 1, case 1: " << (Dist(u, dest, graph) - g)*(cost(u, graph))<< std::endl;
			}
			else
			{
				A[{u, 1, g}] = INF;
				// std::cout<< " q = 1, case 2: "<< INF<< std::endl;
			} 
		}
    }
	
	    for (auto v: R)
	    {
		    // std::cout<< "   Node v:"<<v<< std::endl;
		    if (cost(v) <= cost(u))
		    {
			    indepv[v] = A[{v, (q - 1), 0}] + Dist(u, v)*cost(u);
			    // indepv[v] = Dist(u, v, graph)*cost(u, graph);
		
			    // std::cout<<"    Cost v :"<< cost(v, graph)<<std::endl; 
			    // std::cout<<"    case 1 :"<< indepv[v]<<std::endl; 
		    }
		    else
		    {
		    	indepv[v] = A[{v, (q - 1), (qmax - Dist(u, v))}] + (qmax)*cost(u);
    
	    		// std::cout<<"    Cost v :"<< cost(v, graph)<<std::endl; 
		    	// std::cout<<"    case 2 :"<< indepv[v]<<std::endl; 
		    }
    	}

    	std::sort(R.begin(), R.end(), [&](long v1, long v2) {
	    		return indepv[v1] < indepv[v2];
		    });

    	int i = 0;
	    long first = R[i];

    	for (auto g : gv[u])
	    {
		    if (i == R.size())
		    {
			    A[{u, q, g}] = INF;
			    continue;
		    }        
		    while (true)
		    {
			    if (g <= Dist(u, first) || cost(first) > cost(u))
			    {
				    //std::cout<< "v which halted this while loop :" << first<< std::endl;
				    break;
			    }

		    	i++;    
		
			    if (i == R.size())
			    {
			    break;
			    }

			    first = R[i];
			    //std::cout<<" v now :"<< first<< std::endl;
	    	}
		    if (i == R.size())
	    	{
	    		A[{u, q, g}] = INF;
	    		// std::cout<< " q>1 , case INF: "<< INF<< std::endl;
	    	}
	    	else
	    	{
		    	A[{u, q, g}] = indepv[first] - g * cost(u);
			    // std::cout<< "q>1 , case fill-row: "<< indepv[first] - g*cost(u, graph)<< std::endl; 
		    }   
	    }
    

    // for (const auto& entry : A)
    // {
    //     const Recc& r = entry.first;
    //     long cost = entry.second;
    //   
    //         std::cout << "A[" << r.u << "][" << r.q << "][" << r.g << "] = " << cost << std::endl;   
    // }

    return A;
    }
    
    std::vector<H_Edge> H(long u, long qmax,std::unordered_map<long, std::vector<long>>& gv)
    {
    std::vector<H_Edge> h;
    long dest = 3;
    long curr_node = u;
    std::vector<long> R = Compute_reachable_sets(u, qmax); 

    for (auto v: R)
    {
        for (auto g: gv[u])
        {

                if (cost(v)<= cost(u) && Dist(u, v) <= qmax)
                {
                    h.push_back({u,g,v, 0, (Dist(u,v) - g)*cost(u)});
                }
                else if (cost(v) > cost(u) && Dist(u, v) <= qmax)
                {
                   h.push_back({u, g, v,(qmax - Dist(u,v)), (qmax - g)*cost(u)}); 
                }
                else
                {
                    continue;
                }

        }   
    }
    // Remove elements with negative cost_trav
    h.erase(std::remove_if(h.begin(), h.end(), [](const H_Edge& edge) {
        return edge.cost_trav < 0;
    }), h.end());
    return h;
    }
   
    // Dijkstra's algorithm
    std::vector<long> Dijkstra(const std::vector<H_Edge>& graph, long src, long dest) {
    std::vector<long> dist(graph.size(), INF);
    std::set<std::pair<long, long>> pq; // Pair: (distance, node)
    dist[src] = 0;
    pq.insert({0, src});

    while (!pq.empty()) {
        long u = pq.begin()->second;
        pq.erase(pq.begin());

        for (const H_Edge& edge : graph) {
            if (edge.node_from == u) {
                long v = edge.node_to;
                long weight = edge.cost_trav;

                if (dist[u] + weight < dist[v]) {
                    pq.erase({dist[v], v});
                    dist[v] = dist[u] + weight;
                    pq.insert({dist[v], v});
                }
            }
        }
    }

    return dist;
    }
    
};

int main()
{
    Graph graph;
    graph.addEdge({0, 1, 2, 2});
    graph.addEdge({0, 2, 2, 5});
    graph.addEdge({0, 3, 2, 7});
    graph.addEdge({1, 0, 3, 2});
    graph.addEdge({1, 2, 3, 3});
    graph.addEdge({1, 3, 3, 5});
    graph.addEdge({2, 0, 1, 5});
    graph.addEdge({2, 1, 1, 3});
    graph.addEdge({2, 3, 1, 6});
    graph.addEdge({3, 0, 0, 7});
    graph.addEdge({3, 1, 0, 5});
    graph.addEdge({3, 2, 0, 6});

    long qmax = 6;
    long src=0;
    long dest = 3;
    std::unordered_map<long, std::vector<long>> gv = graph.GV(qmax);
    
     for (auto& g: gv[src])
    {
        g = 0;
    } 
     

//---- UNBOUNDED STOPS-----:
   std::vector<H_Edge> h;
    
    std::vector<H_Edge> h_res;
    
    for (long u=0; u< dest; u++)
    {
        h = graph.H(u, qmax, gv);
        h_res.insert(h_res.end(), h.begin(), h.end());
    }
         
    
    for (auto i: h_res)
    {
         std::cout<< i.node_from<< " : "<< i.gas_at_node_from << " : "<< i.node_to <<" : " << i.gas_left <<" : "<< i.cost_trav<< std::endl;   
    }

    // Find the shortest path from src to dest in h_res
    std::vector<long> shortest_path = graph.Dijkstra(h_res, src, dest);

    // Output the shortest path distance
    if (shortest_path[dest] != INF) {
        std::cout << "Shortest Path from node " << src << " to node " << dest << ": " << shortest_path[dest] << std::endl;
    } else {
        std::cout << "No path exists from node " << src << " to node " << dest << std::endl;
    }
 

//---- WITH STOPS-----------:
    long q_stop = 2;
    std::unordered_map<Recc, long, ReccHash> A;
	std::unordered_map<Recc, long, ReccHash> all_results;

   	for (long u=3; u>=0; u--)
	{
		// std::cout<<"node :"<< u<< std::endl; 
		for (long q=2; q<=q_stop; q++)
		{
			// std::cout<<" q : "<< q << std::endl;  
			A =graph.Fill_Row(u, q, qmax, gv);
			all_results.insert(A.begin(), A.end()); 
		}
	}

   for (const auto& entry : all_results)
    {
        const Recc& r = entry.first;
        long cost = entry.second;
		if (cost>=0 && cost!=INF  && r.u == 0)
		{ 
			std::cout << "A[" << r.u << "][" << r.q << "][" << r.g << "] = " << cost << std::endl;
		}
    } 

    return 0;
}






