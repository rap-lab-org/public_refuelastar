#!/usr/bin/env python
from itertools import product
import pandas as pd


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


if __name__ == "__main__":
    gen_city()
