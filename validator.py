#!/usr/bin/env python
import sys
import os
import pandas as pd


def check(df: pd.DataFrame):
    idxs = ['map', 's', 't', 'K', 'Q']
    t = df.drop(columns=['size', 'runtime']).pivot_table(index=idxs, 
                                                         columns='algo', 
                                                         values='best')
    n, _ = t[t['dp'] != t['erca']].shape
    if n > 0:
        print(t[t['dp'] != t['erca']])
        # assert False

    n, _ = t[t['dp'] != t['erca-noh']].shape
    if n > 0:
        print(t[t['dp'] != t['erca-noh']])

    if 'mip' in t.columns:
        n, _ = t[t['dp'] != t['mip']].shape
        if n > 0:
            print(t[t['dp'] != t['mip']])

def check_all(dirname: str):
    logs = [i for i in os.listdir(dirname) if i.endswith(".log")]
    for log in logs:
        logpath = os.path.join(dirname, log)
        print (f"Checking {logpath} ...")
        df = pd.read_csv(logpath)
        check(df)
    print ("All passed.")


if __name__ == "__main__":
    check_all(sys.argv[1])
