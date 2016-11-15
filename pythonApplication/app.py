from tkinter import *
import os

dir_path = os.path.dirname(os.path.realpath(__file__))
dir_path_string = str(dir_path)

root = Tk()
theLabel = Label(root, text="This is too easy")
pathLabel = Label(root, text=dir_path_string)
theLabel.pack()
pathLabel.pack()


root.mainloop() # runs our application indefinitely until closed
