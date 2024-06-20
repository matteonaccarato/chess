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
        "title": "RANDOM (PL1) vs MINIMAX (PL2)",
        "df": []
    },
    "minimax_minimax": {
        "filename": "minimax_minimax.csv",
        "title": "MINIMAX (PL1) vs MINIMAX (PL2)",
        "df": []
    },
    "minimax_base_minimax_pesto": {
        "filename": "minimax_base_minimax_pesto.csv",
        "title": "MINIMAX BASE (PL1) vs MINIMAX PeSTO (PL2)",
        "df": []
    }
}

# LOAD DATAFRAMES FROM CSV
dfs = []
for value in file_data.values():
  value["df"] = pd.read_csv(base_url + value["filename"])

# PLOT DATA
for idx, data in enumerate(file_data.values()):
  fig = px.line(data["df"],
              x="#GAME", y = ["PL_1_WINS/GAMES", "PL_2_WINS/GAMES", "DRAWS/GAMES"],
              title= "<b>" + str(idx + 1) + ") " + data["title"] + "</b>",
              labels = { 
                  "#GAME": "number of games",
                  "value": "<b>GAMES RATIO</b>", 
                  "variable": "DATA"
              })
  fig.show()