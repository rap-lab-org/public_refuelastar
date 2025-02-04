#!/usr/bin/env python
from itertools import product
import os
import pandas as pd
import numpy as np


def gen_queries(fn: str, n: int, seed: int = 0):
    import random

    random.seed(seed)

    df = pd.read_csv(fn).rename(columns=lambda x: x.strip())
    ids = set(df['index_from']) | set(df['index_to'])
    all_pairs = list([(i ,j) for i, j in product(list(ids), list(ids)) if i != j])
    queries = random.sample(all_pairs, k=n)

    query_fn = fn.removesuffix(".csv") + ".query"
    with open(query_fn, "w") as f:
        f.write("from,to\n")
        for i, j in queries:
            f.write(f"{i},{j}\n")


def gen_city():
    exprs = [
        "./city-data/Austin_gas.csv",
        "./city-data/London_gas.csv",
        "./city-data/Moscow_no_zero_dist.csv",
        "./city-data/Phil_gas.csv",
        "./city-data/Phoenix_gas.csv",
    ]
    n = 100
    for expr in exprs:
        gen_queries(expr, n)
         

def gen_random_graph(n: int=10, elb: float = 1, eub: float = 9, seed: int=0):
    import networkx as nx
    g = nx.gnp_random_graph(n, 0.3, seed=seed)
    cs = list(nx.connected_components(g))
    for u, v in g.edges:
        w = int(np.random.uniform(elb, eub))
        g.add_edge(u, v, weight=w)
    for i in range(len(cs)-1):
        u = min(cs[i])
        v = min(cs[i+1])
        g.add_edge(u, v, weight=eub) 
        g.add_edge(u, v, weight=eub)
    assert (len(list(nx.connected_components(g))) == 1)
    return g

def synthetic_map(n: int=10, qmax: int = 10):
    g = gen_random_graph(n, 1, min(8, qmax))
    cost = {}
    for v in g.nodes:
        cost[v] = int(np.random.uniform(1, 10))
    # Intentionally add extra whitespace after the name
    headers = ['Gas_node_from ', 'Gas_node_to ', 'distance ', 'Cost ', 'index_from ', 'index_to']
    data = {'Gas_node_from ': [],
            'Gas_node_to ': [],
            'distance ': [],
            'Cost ': [],
            'index_from ': [],
            'index_to': []}
    for u, v in g.edges:
        w = g.get_edge_data(u, v)['weight']
        price = cost[u]
        data['Gas_node_from '].append(u)
        data['Gas_node_to '].append(v)
        data['distance '].append(w)
        data['Cost '].append(price)
        data['index_from '].append(u)
        data['index_to'].append(v)

    for u, v in g.edges:
        w = g.get_edge_data(u, v)['weight']
        price = cost[v]
        data['Gas_node_from '].append(v)
        data['Gas_node_to '].append(u)
        data['distance '].append(w)
        data['Cost '].append(price)
        data['index_from '].append(v)
        data['index_to'].append(u)
    df = pd.DataFrame(data, columns=headers)
    return df

def gen_synthetic():
    qmax = 10
    dirname = "./syn-data"
    if not os.path.exists(dirname):
        os.makedirs(dirname, exist_ok=True)
    for n in [8, 16, 32]:
        df = synthetic_map(n, qmax)
        fpath = f"{dirname}/{n}.csv"
        df.round(2).to_csv(open(fpath, 'w'), index=False)
        if n > 10:
            gen_queries(fpath, 100)

    for n in [256, 512, 1024]:
        df = synthetic_map(n, qmax)
        fpath = f"{dirname}/{n}.csv"
        df.to_csv(open(fpath, 'w'), index=False)
        gen_queries(fpath, 100)

if __name__ == "__main__":
    gen_city()
    gen_synthetic()
