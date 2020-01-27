import matplotlib.pyplot as plt
import numpy as np
import tkinter as tk


def main():
    config_file = open('config.txt', 'r+')
    lines = config_file.readlines()
    settings = AccelSettings(lines)
    vels = np.arange(0, 5, .1)
    sens = np.array([quake_accel(vel, settings) for vel in vels])
    plt.plot(vels, sens)
    plt.show()

    window = tk.Tk()
    window.title('Marley Accel Setup')
    window.geometry('800x600')

    heading = tk.Label(window, text='Settings')
    heading.grid(row=0, column=1)

    field_names = [
        'Base Sens', 'Accel', 'Offset', 'Upper Bound', 'Power', 'Game Sens',
        'Pre-Scalar-X', 'Pre-Scalar-Y', 'Post-Scalar-X', 'Post-Scalar-Y'
    ]

    for idx, field_name in enumerate(field_names):
        base = tk.Label(window, text=field_name)
        base.grid(row=idx + 1, column=0)
        base_entry = tk.Entry(window)
        base_entry.grid(row=idx + 1, column=1, ipadx='100')

    submit = tk.Button(window,
                       text='Apply',
                       fg='Black',
                       bg='Red',
                       command=submit_fields)
    submit.grid(row=len(field_names) + 1, column=1)
    print(window)

    window.mainloop()


def submit_fields():
    print('hi')


class AccelSettings:
    def __init__(self, config_strs):
        self.settings = AccelSettings.default()
        splt = [s.strip().split("=") for s in config_strs]
        user_settings = dict([(s[0], float(s[1])) for s in splt])
        for key in user_settings:
            self.settings[key] = user_settings[key]

    @staticmethod
    def default():
        return {
            'base': 1,
            'offset': 0,
            'upper_bound': 128,
            'accel_rate': 1,
            'power': 1,
            'game_sens': 1
        }

    def __getitem__(self, key):
        return self.settings[key]


def quake_accel(velocity, settings):
    """
    Re-implementation of quake_accel. Easier than making python bindings
    Input is mouse velocity. 
    """
    change = max(velocity - settings['offset'], 0)
    p = (settings['accel_rate'] * change)**(settings['power'] - 1)
    unbounded = settings['base'] + p
    bounded = min(unbounded, settings['upper_bound'])
    accel_sens = bounded / settings['game_sens']
    return accel_sens


if __name__ == '__main__':
    main()
