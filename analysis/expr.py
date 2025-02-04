# %%
import pandas as pd
import seaborn as sns

# %%
def get_station_num(city: str):
  city_file = f"../city-data/{city}.csv"
  with open(city_file, 'r') as f:
    raw_lines = f.readlines()
    ids = set()
    for i in raw_lines[1:]:
      idfrom = i.split(',')[-2].strip()
      idto = i.split(',')[-2].strip()
      ids.add(idfrom)
      ids.add(idto)
  return len(ids)

# %%
exprs = [
  "../output/Austin_gas.log",
  "../output/London_gas.log",
  "../output/Phil_gas.log",
  "../output/Phoenix_gas.log",
  "../output/Moscow_no_zero_dist.log",
]
snums = {}
for expr in exprs:
  name = expr.split('/')[-1].removesuffix('.log')
  num = get_station_num(name)
  snums[name] = num
headers = 'map,s,t,K,Q,algo,best,size,runtime,init_time'.split(',')
df = pd.concat([pd.read_csv(expr,skiprows=[0], names=headers) for expr in exprs])
df['stations'] = df['map'].map(snums)
df.head()

# %%
index = ['map', 's', 't', 'K', 'Q']
erca = df[df['algo'] == 'erca'].set_index(index).drop(columns=['algo', 'best'])
dp = df[df['algo'] == 'dp'].set_index(index).drop(columns=['algo', 'best'])

# %%
erca['ratio'] = erca['init_time'] / erca['runtime']
erca['ratio'].describe().to_frame().T.round(4)

# %%
(dp / erca).groupby('map')[['size', 'runtime']].describe().round(2)

# %%
dp['speedup'] = dp['runtime'] / erca['runtime']
df['runtime_ms'] = df['runtime'] / 1e3
speedup = dp.reset_index().drop(columns=['s','t','K','Q','size','init_time'])
speedup.head()

# %%
# tmp = df[df['algo'] == 'dp']
# tmp.groupby('map')['runtime'].describe().round(2)

ax2 = sns.lineplot(data=speedup, x='stations', y='speedup', color='green', alpha=0.5, label='dp/erca',
             linewidth=1)
ax2.set_ylim(1, 5)
ax = ax2.twinx()
sns.lineplot(ax=ax, data=df, x='stations', y='runtime_ms', hue='algo', style='algo', markers=True)
# ax.set_yscale('log')
# ax.set_xscale('log')
ax.set_ylabel("Runtime (ms)")
ax2.legend(loc='upper left')
ax.legend(loc='lower right')

# %%
ax = sns.lineplot(data=df, x='stations', y='size', hue='algo', style='algo', markers=True)
ax.set_yscale('log')
ax.set_ylabel("Size")


