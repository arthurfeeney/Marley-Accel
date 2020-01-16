import matplotlib.pyplot as plt
import numpy as np


def main():
    config_file = open('config.txt', 'r+')
    lines = config_file.readlines()
    settings = AccelSettings(lines)
    vels = np.arange(0, 5, .1)
    sens = np.array([quake_accel(vel, settings) for vel in vels])
    plt.plot(vels, sens)
    plt.show()


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
