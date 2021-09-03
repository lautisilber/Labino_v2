from tkinter import Label
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import json
from os import listdir
from os.path import join, isfile
from datetime import datetime

''' credit to https://stackoverflow.com/questions/31410043/hiding-lines-after-showing-a-pyplot-figure '''

def show_plot(logs_folder: str):
    log_files = [join(logs_folder, f) for f in listdir(logs_folder) if isfile(join(logs_folder, f))]
    dates = []
    moists = []
    hums = []
    temps = []
    for file in log_files:
        with open(file, 'r') as f:
            for line in f.readlines():
                try:
                    log = json.loads(line)
                    if 'time' in log and 'soil' in log and 'hum' in log and 'temp' in log:
                        dates.append(datetime.strptime(log['time'], '%Y-%m-%d %H:%M:%S'))
                        moists.append([m for m in log['soil']])
                        hums.append(log['hum'])
                        temps.append(log['temp'])
                except:
                    print('skipped log')
    moists = list(map(list, zip(*moists)))
    dates.sort()
    hums = sort_by_dates(hums, dates)
    temps = sort_by_dates(temps, dates)
    moists = [sort_by_dates(m, dates) for m in moists]

    fig, ax = plt.subplots(figsize=(8, 4.8))
    for i, m in enumerate(moists):
        ax.plot(dates, m, label=f'sensor {i}')
    ax.plot(dates, hums, label='hums')
    ax.plot(dates, temps, label='temps')
    ax.legend(loc='upper left', bbox_to_anchor=(1.05, 1), borderaxespad=0)
    fig.subplots_adjust(right=0.8)
    fig.suptitle('Click legend to hide')
    leg = interactive_legend()
    plt.show()

def sort_by_dates(l, dates):
    a = list(zip(l, dates))
    a.sort(key=lambda x: x[1])
    a = [e[0] for e in a]
    return a

def interactive_legend(ax=None):
    if ax is None:
        ax = plt.gca()
    if ax.legend_ is None:
        ax.legend()

    return InteractiveLegend(ax.get_legend())

class InteractiveLegend(object):
    def __init__(self, legend):
        self.legend = legend
        self.fig = legend.axes.figure

        self.lookup_artist, self.lookup_handle = self._build_lookups(legend)
        self._setup_connections()

        self.update()

    def _setup_connections(self):
        for artist in self.legend.texts + self.legend.legendHandles:
            artist.set_picker(10) # 10 points tolerance

        self.fig.canvas.mpl_connect('pick_event', self.on_pick)
        self.fig.canvas.mpl_connect('button_press_event', self.on_click)

    def _build_lookups(self, legend):
        labels = [t.get_text() for t in legend.texts]
        handles = legend.legendHandles
        label2handle = dict(zip(labels, handles))
        handle2text = dict(zip(handles, legend.texts))

        lookup_artist = {}
        lookup_handle = {}
        for artist in legend.axes.get_children():
            if artist.get_label() in labels:
                handle = label2handle[artist.get_label()]
                lookup_handle[artist] = handle
                lookup_artist[handle] = artist
                lookup_artist[handle2text[handle]] = artist

        lookup_handle.update(zip(handles, handles))
        lookup_handle.update(zip(legend.texts, handles))

        return lookup_artist, lookup_handle

    def on_pick(self, event):
        handle = event.artist
        if handle in self.lookup_artist:

            artist = self.lookup_artist[handle]
            artist.set_visible(not artist.get_visible())
            self.update()

    def on_click(self, event):
        if event.button == 3:
            visible = False
        elif event.button == 2:
            visible = True
        else:
            return

        for artist in self.lookup_artist.values():
            artist.set_visible(visible)
        self.update()

    def update(self):
        for artist in self.lookup_artist.values():
            handle = self.lookup_handle[artist]
            if artist.get_visible():
                handle.set_visible(True)
            else:
                handle.set_visible(False)
        self.fig.canvas.draw()

    def show(self):
        plt.show()