#!/usr/bin/python3
import matplotlib.pyplot as plt
import pandas as pd
import sys
import argparse
import re
import os
import glob

path = ""

FOUND_DEFAULT = True


# def getMostRecentBenchmarkFile():
#     try:
#         root = "build/output/"
#         files = list(filter(os.path.isfile, glob.glob(root + "*.csv")))
#         files.sort(key=lambda x: os.path.getmtime(x), reverse=True)
#         return files[0]
#     except:
#         return ""

# files = [i for i in files if i.endswith(
#     ".csv") and i.startswith("benchmark")]

# filesAndStamps = {}

# for i in files:
#     m = re.search(
#         r"(?<=benchmark_)(\d+)-(\d+)-(\d+)_(\d+)-(\d+)-(\d+)(?=\.csv)", i)

#     if m:
#         timestamp = (int(m.group(1)) - 1970) * 31556926
#         timestamp += int(m.group(2)) * 2629743
#         timestamp += int(m.group(3)) * 86400
#         timestamp += int(m.group(4)) * 3600
#         timestamp += int(m.group(5)) * 60
#         timestamp += int(m.group(6))

#         filesAndStamps[i] = timestamp

# if (len(filesAndStamps) == 0):
#     print("No benchmark files found")
#     sys.exit(1)

# mostRecent = max(filesAndStamps.values())
# mostRecentFile = [k for k, v in filesAndStamps.items()
#                   if v == mostRecent][0]

# return os.path.join(root, mostRecentFile)


parser = argparse.ArgumentParser(
    description='Plot benchmark results from a csv file')

parser.add_argument('graphs', type=str, nargs='+',
                    help='The columns to graph in the format "title, x, y1, y2, ...". Optionally, you can add xLabel= and yLabel= to specify the labels for the x and y axis.')

parser.add_argument('--file', type=str, nargs='*', required=True,
                    help='The csv file(s) to read the data from')

parser.add_argument('--xOffset', type=int, default=0,
                    help='The offset to add to the x axis')


class ListAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if values == None or len(values) == 0:
            values = namespace.file

        if type(values) == str:
            values = [values]

        dfs = []
        for path in values:
            dfs.append(pd.read_csv(path))

        for df in dfs:
            df.rename(columns=lambda x: x.strip(), inplace=True)

        for i in range(len(dfs)):
            print(f"File {i}: {values[i]}")
            print(dfs[i].columns)

        parser.exit()


parser.register('action', 'list', ListAction)
parser.add_argument('--list', action='list',
                    help='List the columns in the files provided and exit',
                    nargs='*')

args = parser.parse_args()

paths = args.file

if (len(paths) == 0):
    print("No file provided and no default file found")
    sys.exit(1)

# df = pd.read_csv(path)

dfs = []
for path in paths:
    dfs.append(pd.read_csv(path))

for df in dfs:
    df.rename(columns=lambda x: x.strip(), inplace=True)


# plot the data, with x-axis as the time and y axis as the rest of the columns, excluding the first one
# label the y axis with the column names
# print(df.columns)
# df.plot(x='time', y=df.columns[2:], title="Benchmark Results")
# plt.ylabel("App Time (s)")
# plt.xlabel("Time (ms)")
# plt.show()


def plot(dfs, argDf, title, x, *y, yLabels, xLabel=None, yLabel=None):
    f = plt.figure()
    # df.plot(x=x, y=list(y), title=title, ax=f.gca())
    series = []
    for i in range(len(y)):
        series.append(dfs[argDf[i]][y[i]][args.xOffset:])

    for i in range(len(y)):
        plt.plot(dfs[argDf[i]][x][args.xOffset:],
                 series[i], label=yLabels[i], )

    plt.title(title)
    plt.legend()

    if (yLabel):
        plt.ylabel(yLabel)
    if (xLabel):
        plt.xlabel(xLabel)

    f.show()
    return f


def getStringLikeliness(string, target):
    return sum([1 if i in target else 0 for i in string]) / max(len(string), len(target))


def getMostSimilarString(string, list):
    return max(list, key=lambda x: getStringLikeliness(string, x))


def getMostSimilarColumnOrNone(string, df):
    columns = df.columns
    mostSimilar = getMostSimilarString(string, columns)
    if (getStringLikeliness(string, mostSimilar) > 0.5):
        return mostSimilar
    return None


def existsIdenticalColumnButDifferentCase(string, df):
    columns = df.columns
    for i in columns:
        if (i.lower() == string.lower()):
            return i
    return None


for graph in args.graphs:
    arguments = graph.split(',')
    if (len(arguments) < 3):
        print("Error: Not enough arguments")
        sys.exit(1)
    title = arguments[0].strip()
    x = arguments[1].strip().split("[")[0].strip()
    Y = arguments[2:]
    yLabelsSpecified = [""] * len(Y)
    for i, y in enumerate(Y):
        if ":" in y:
            yLabelsSpecified[i] = y.split(":")[1].strip()
            Y[i] = y.split(":")[0].strip()

    y = [i.strip().split("[")[0].strip() for i in Y]
    labelX = None
    labelY = None
    toRemove = []
    for i, yy in enumerate(y):
        iLower = yy.lower()
        if re.match("(label)?x(label)?=", iLower):
            labelX = yy.split('=')[1]
            toRemove.append(yy)
        elif re.match("(label)?y(label)?=", iLower):
            labelY = yy.split('=')[1]
            toRemove.append(yy)

    for yy in toRemove:
        y.remove(yy)

    yDfs = []
    for yy in arguments[2:]:
        if ("[" in yy):
            yDfs.append(int(yy.split("[")[1].split("]")[0]))
        else:
            yDfs.append(0)

    # check if the columns are valid
    for yDf in yDfs:
        if (x not in dfs[yDf].columns):
            identicalColumn = existsIdenticalColumnButDifferentCase(
                x, dfs[yDf])
            if (identicalColumn):
                x = identicalColumn
            else:
                print(f"Error: {x} is not a valid column")
                mostSimilar = getMostSimilarColumnOrNone(x, dfs[yDf])
                if (mostSimilar):
                    print(f"Did you mean {mostSimilar}?")
                sys.exit(1)

    for yy in range(len(y)):
        if (y[yy] not in dfs[yDfs[yy]].columns):
            identicalColumn = existsIdenticalColumnButDifferentCase(
                y[yy], dfs[yDfs[yy]])
            if (identicalColumn):
                y[y.index(y[yy])] = identicalColumn
            else:
                print(f"Error: {y[yy]} is not a valid column")
                mostSimilar = getMostSimilarColumnOrNone(y[yy], dfs[yDfs[yy]])
                if (mostSimilar):
                    print(f"Did you mean {mostSimilar}?")
                sys.exit(1)

    dfIndices = yDfs

    if (len(args.file) == 1):
        ylabels = [f"{y[i]}" for i in range(len(y))]
    else:
        ylabels = [
            f"{y[i]} (fichier {os.path.split(args.file[dfIndices[i]])[-1]})" for i in range(len(y))]

    for yy, label in enumerate(yLabelsSpecified):
        if (label):
            ylabels[yy] = label

    f = plot(dfs, dfIndices, title, x, *y,
             yLabels=ylabels, xLabel=labelX, yLabel=labelY)


while (any([plt.fignum_exists(i) for i in plt.get_fignums()])):
    plt.pause(0.1)
