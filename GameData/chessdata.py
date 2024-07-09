# -*- coding: utf-8 -*-
"""ChessData
    A simple (using pandas and plotly) to show bot performance data against each other.
    https://colab.research.google.com/drive/1uGegbpx3qINZf54UFUeeekP_x8Yc4p6_
"""

import pandas as pd
import plotly.express as px

# INIT
base_url = "https://github.com/matteo-naccarato/chess/raw/main/GameData/"
file_data = {
    "random_random": {
        "filename": "random_random.csv",
        "title": "RANDOM (PL1) vs RANDOM (PL2)",
        "df": []
    },
    "random_minimax": {
        "filename": "random_minimax.csv",
        "title": "RANDOM (PL1) vs MINIMAX BASE (PL2)",
        "df": []
    },
    "minimax_minimax": {
        "filename": "minimax_minimax.csv",
        "title": "MINIMAX BASE (PL1) vs MINIMAX BASE (PL2)",
        "df": []
    },
    "minimax_base_minimax_pesto": {
        "filename": "minimax_base_minimax_pesto.csv",
        "title": "MINIMAX BASE (PL1) vs MINIMAX peSTO (PL2)",
        "df": []
    }
}

trace_labels = {
    "PL_1_WINS/GAMES": ["Random WINS", "Random WINS", "Minimax Base WINS", "Minimax Base WINS"],
    "PL_2_WINS/GAMES": ["Random WINS", "Minimax Base WINS", "Minimax Base WINS", "Minimax peSTO WINS"],
    "DRAWS/GAMES": ["DRAWS", "DRAWS", "DRAWS", "DRAWS"],
}
# LOAD DATAFRAMES FROM CSV
dfs = []
for value in file_data.values():
  value["df"] = pd.read_csv(base_url + value["filename"])

# PLOT DATA
fig_idx = 0
trace_idx = 0
def get_trace_label(t_name):
  global fig_idx, trace_idx
  if trace_idx % 3 == 0:
    # New Figure
    fig_idx += 1
    trace_idx = 0
  trace_idx += 1
  return trace_labels[t_name][fig_idx - 1]


for idx, data in enumerate(file_data.values()):
  fig = px.line(data["df"],
              x="#GAME", y = ["PL_1_WINS/GAMES", "PL_2_WINS/GAMES", "DRAWS/GAMES"],
              title= "<b>" + str(idx + 1) + ") " + data["title"] + "</b>",
              labels = {
                  "#GAME": "Number of Games",
                  "value": "<b>GAMES RATIO</b>",
                  "variable": "<b>DATA (RATIOS)</b>"
              },
              hover_data = {
                  "#GAME": False
              }).for_each_trace(lambda t: 
                  t.update(name = get_trace_label(t.name))
              ).update_layout(hovermode="x")
  fig.show()