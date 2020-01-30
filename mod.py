import argparse
import numpy as np
import tkinter as tk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg,
                                               NavigationToolbar2Tk)

parser = argparse.ArgumentParser(
    description="Configure your configuration settings.")
parser.add_argument("config_file_path")
args = parser.parse_args()


def main():
    global args
    config_file_name = args.config_file_path

    window = tk.Tk()
    window.title('Marley Accel Setup')
    window.geometry('1200x600')

    heading = tk.Label(window, text='Settings')
    heading.grid(row=0, column=1)

    entries = {}
    field_names = list(DefaultSettings().get().keys())

    for idx, field_name in enumerate(field_names):
        base = tk.Label(window, text=field_name)
        base.grid(row=idx + 1, column=0)
        entries[field_name] = tk.Entry(window)
        entries[field_name].grid(row=idx + 1, column=1, ipadx='100')

    init_entries(entries, config_file_name)

    reset = tk.Button(window,
                      text='Reset',
                      fg='black',
                      bg='red',
                      command=reset_entries(entries,
                                            DefaultSettings().get()))
    reset.place(x=180, y=225)

    submit = tk.Button(window,
                       text='Apply',
                       fg='Black',
                       bg='Red',
                       command=submit_entries(config_file_name, entries,
                                              field_names, draw_accel_plot,
                                              window))
    submit.place(x=250, y=225)

    # map entries to dict of name to value. Use to draw accel plot
    settings = entries_to_dict(entries)
    draw_accel_plot(window, settings)

    window.mainloop()


def init_entries(entries, config_file_name):
    """
    initialize entries to values in the config file
    """
    with open(config_file_name, 'r') as config_file:
        lines = config_file.readlines()
        config_map = dict([line.split('=') for line in lines])
        for name in entries.keys():
            entries[name].delete(0, tk.END)
            entries[name].insert(0, config_map[name].strip())


def entries_to_dict(entries):
    """
    Convert the dict containing entries to a dict mapping to entries' values. 
    """
    settings = [(key, entry.get()) for (key, entry) in entries.items()]
    settings = dict(settings)
    for name in settings.keys():
        # if the setting can't be converted to float, use the default.
        try:
            settings[name] = float(settings[name])
        except ValueError:
            settings[name] = float(DefaultSettings().get()[name])
    return settings


def draw_command(window, entries):
    """
    Wrapper for function that draw the plot, so it can be used as a 
    command. 
    """
    settings = entries_to_dict(entries)

    def draw_func():
        draw_accel_plot(window, settings)

    return draw_func


def draw_accel_plot(window, settings):
    fig = Figure(dpi=100, tight_layout=False)
    fig.suptitle("Acceleration Grids")
    rate = np.arange(0, 25, .1)
    accel_sens = [
        simple_accel(rate[i], settings) for i in range(rate.shape[0])
    ]
    fig.add_subplot(111, xlabel='Mouse Velocity',
                    ylabel='Sensitivity').plot(rate, accel_sens)
    canvas = FigureCanvasTkAgg(fig, master=window)
    canvas.draw()
    canvas.get_tk_widget().place(x=450, y=10)


def reset_entries(entries, default_settings):
    def reset_func():
        for name in default_settings:
            entries[name].delete(0, tk.END)
            entries[name].insert(0, default_settings[name])

    return reset_func


def submit_entries(config_file_name, entries, names, draw_func, window):
    def submission():
        # write entries to config file.
        with open(config_file_name, 'w+') as config_file:
            for name in names:
                config_file.write(name + '=' + entries[name].get() + '\n')
        # redraw the accel plot
        settings = entries_to_dict(entries)
        draw_func(window, settings)

    return submission


class DefaultSettings:
    """
    Stronger guarantee that default settings won't be changed during 
    program execution. 
    """

    def __init__(self):
        self.__default_settings = {
            'base': 1.0,
            'offset': 0.0,
            'upper_bound': 10000.0,  # arbitrary large float
            'accel_rate': 0.0,
            'power': 2.0,
            'game_sens': 1.0,
            'overflow_lim': 127,  # C signed char max
            'pre_scalar_x': 1.0,
            'pre_scalar_y': 1.0,
            'post_scalar_x': 1.0,
            'post_scalar_y': 1.0
        }

    def get(self):
        return self.__default_settings.copy()


def simple_accel(rate, args):
    # apply acceleration to mouse movements rate of change
    # ignores pre- and post-scalars
    change = max(rate - args['offset'], 0.0)
    unbound = args['base'] + (args['accel_rate'] * change)**(args['power'] - 1)
    bound = min(unbound, args['upper_bound'])
    sens = bound / args['game_sens']
    return sens


if __name__ == '__main__':
    main()
