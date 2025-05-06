import numpy as np
import matplotlib.pyplot as plt

def plot_heatmap(filename="output.txt"):
    data = np.loadtxt(filename)
    plt.imshow(data, cmap='hot', interpolation='nearest')
    plt.colorbar(label='Temperature (°C)')
    plt.title("Heat Diffusion Final State")
    plt.show()

if __name__ == "__main__":
    plot_heatmap()
