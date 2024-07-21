import osmnx as ox
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import multiprocessing
import sys
import resource

place_name = "Pennsylvania, USA"
graph = ox.graph_from_place(place_name, network_type = "drive", retain_all=True)

gdf = ox.features_from_place(place_name, tags={'amenity': 'fuel'})

gdf_proj = gdf.to_crs(graph.graph['crs'])


nearest_nodes_dict = {}
for index, row, in gdf.iterrows():
    centroid =row['geometry'].centroid
    lng = centroid.x
    lat = centroid.y
    nearest_node = ox.distance.nearest_nodes(graph, lng, lat)
    #print("index :", index, " nearest node :", nearest_node)
    #print(index[0])
    if (index[0]=="node"):
        nearest_nodes_dict[index[1]]=nearest_node

count = 0
for i in nearest_nodes_dict:
    count +=1

def memory_limit():
    """Limit max memory usage to half."""
    soft, hard = resource.getrlimit(resource.RLIMIT_AS)
    # Convert KiB to bytes, and divide in two to half
    resource.setrlimit(resource.RLIMIT_AS, (get_memory() * 1024 // 2, hard))

def get_memory():
    with open('/proc/meminfo', 'r') as mem:
        free_memory = 0
        for i in mem:
            sline = i.split()
            if str(sline[0]) in ('MemFree:', 'Buffers:', 'Cached:'):
                free_memory += int(sline[1])
    return free_memory  # KiB


def calculate_route_length(orig, dest, graph):
    route = ox.shortest_path(graph, orig, dest, weight="length")
    route_length = 0
    if route is not None:
        for i in range(len(route) - 1):
            u, v = route[i], route[i + 1]
            if graph.has_edge(u, v) and 'length' in graph[u][v][0]:
                route_length += graph[u][v][0]['length']
    return route_length

def process_nearest_nodes(nearest_nodes_dict, graph):
    df = pd.DataFrame(columns=['Gas_node_from', 'Gas_node_to', 'distance'])
    for i in nearest_nodes_dict:
        orig = nearest_nodes_dict[i]
        for j in nearest_nodes_dict:
            if i != j:
                dest = nearest_nodes_dict[j]
                route_length = calculate_route_length(orig, dest, graph)
                df = df.append({'Gas_node_from': i, 'Gas_node_to': j, 'distance': route_length}, ignore_index=True)
    df['distance'] = df['distance'].astype(float)
    return df

if __name__ == '__main__':
    # Convert nearest_nodes_dict to a list of tuples
    nearest_nodes_list = list(nearest_nodes_dict.items())

    # Split nearest_nodes_list into chunks based on the number of CPU cores
    num_cores = multiprocessing.cpu_count()
    chunk_size = len(nearest_nodes_list) // num_cores
    chunks = [nearest_nodes_list[i:i + chunk_size] for i in range(0, len(nearest_nodes_list), chunk_size)]

    # Initialize a multiprocessing pool with the number of CPU cores
    pool = multiprocessing.Pool(processes=num_cores)

    # Calculate route lengths in parallel using multiprocessing
    results = pool.starmap(process_nearest_nodes, [(dict(chunk), graph) for chunk in chunks])

    # Close the pool to free resources
    pool.close()
    pool.join()

    # Concatenate the dataframes from each process
    df = pd.concat(results, ignore_index=True)

    df.to_csv(f"{place_name}_gas_stations.csv", index=False)

'''
    BELOW CODE TRIES TO LIMIT THE RAM USAGE SUCH THAT MY PC DOESN'T CRASH, BUT IT GOES TO THE "EXCEPT" ISSUE EVERYTIME
'''


# import osmnx as ox
# import numpy as np
# import matplotlib.pyplot as plt
# import pandas as pd
# import multiprocessing
# import sys
# import resource

# # # Function to calculate the available memory in bytes
# # def get_free_memory():
# #     free_memory = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss * resource.getpagesize()
# #     return free_memory

# def memory_limit():
#     """Limit max memory usage to half."""
#     soft, hard = resource.getrlimit(resource.RLIMIT_AS)
#     # Convert KiB to bytes, and divide in two to half
#     resource.setrlimit(resource.RLIMIT_AS, (get_memory() * 1024 / 2, hard))

# def get_memory():
#     with open('/proc/meminfo', 'r') as mem:
#         free_memory = 0
#         for i in mem:
#             sline = i.split()
#             if str(sline[0]) in ('MemFree:', 'Buffers:', 'Cached:'):
#                 free_memory += int(sline[1])
#     return free_memory  # KiB

# # Function to calculate the route length between two nodes
# def calculate_route_length(graph, orig, dest):
#     route = ox.shortest_path(graph, orig, dest, weight="length")
#     route_length = 0
#     if route is not None:
#         for i in range(len(route) - 1):
#             u, v = route[i], route[i + 1]
#             if graph.has_edge(u, v) and 'length' in graph[u][v][0]:
#                 route_length += graph[u][v][0]['length']
#     return route_length

# # Function to process nearest nodes
# def process_nearest_nodes(nearest_nodes_dict, graph):
#     df = pd.DataFrame(columns=['Gas_node_from', 'Gas_node_to', 'distance'])
#     for i in nearest_nodes_dict:
#         orig = nearest_nodes_dict[i]
#         for j in nearest_nodes_dict:
#             if i != j:
#                 dest = nearest_nodes_dict[j]
#                 route_length = calculate_route_length(graph, orig, dest)
#                 df = df.append({'Gas_node_from': i, 'Gas_node_to': j, 'distance': route_length}, ignore_index=True)
#     df['distance'] = df['distance'].astype(float)
#     return df

# def main():
#     # Load the OSM data using pyrosm
#     place_name = "Pennsylvania, USA"
#     graph = ox.graph_from_place(place_name, network_type="drive")
#     # Fetch the fuel stations data using pyrosm
#     gdf = ox.features_from_placee(place_name, tags={'amenity': 'fuel'})

#     nearest_nodes_dict = {}
#     for index, row in gdf.iterrows():
#         centroid = row['geometry'].centroid
#         lng = centroid.x
#         lat = centroid.y
#         nearest_node = ox.distance.nearest_nodes(graph, lng, lat)
#         if index[0] == "node":
#             nearest_nodes_dict[index[1]] = nearest_node

#     # Convert nearest_nodes_dict to a list of tuples
#     nearest_nodes_list = list(nearest_nodes_dict.items())

#     # Split nearest_nodes_list into chunks based on the number of CPU cores
#     num_cores = multiprocessing.cpu_count()
#     chunk_size = len(nearest_nodes_list) // num_cores
#     chunks = [nearest_nodes_list[i:i + chunk_size] for i in range(0, len(nearest_nodes_list), chunk_size)]

#     # Initialize a multiprocessing pool with the number of CPU cores
#     pool = multiprocessing.Pool(processes=num_cores)

#     # Calculate route lengths in parallel using multiprocessing
#     results = pool.starmap(process_nearest_nodes, [(dict(chunk), graph) for chunk in chunks])

#     # Close the pool to free resources
#     pool.close()
#     pool.join()

#     # Concatenate the dataframes from each process
#     df = pd.concat(results, ignore_index=True)

#     df.to_csv(f"{place_name}_gas_stations.csv", index=False)
    
# if __name__ == '__main__':
#     # Load the OSM data using pyrosm
#     # place_name = "Pennsylvania, USA"
#     # graph = ox.graph_from_place(place_name, network_type="drive", retain_all=True)

#     # # Get the free memory in bytes
#     # free_memory = get_free_memory()

#     # Set resource limit for memory usage to half of the free memory
#     #resource.setrlimit(resource.RLIMIT_AS, (free_memory // 2, free_memory // 2))

#     memory_limit()
#     try:
#         main()
#     except MemoryError:
#         sys.stderr.write('\n\nERROR: Memory Exception\n')
#         sys.exit(1)
    
