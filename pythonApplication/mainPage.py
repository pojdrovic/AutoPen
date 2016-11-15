'''
from tkinter import Tk, BOTH, Label
from tkinter.ttk import Frame, Button, Style
#from PIL import Image, ImageTk

''''


from tkinter import Tk, Frame, Checkbutton, Button
from tkinter import BooleanVar, BOTH
from git import Repo
import git
import os


class Example(Frame):

    def __init__(self, parent):
        Frame.__init__(self, parent)

        self.parent = parent

        self.initUI()


    def initUI(self):

        self.parent.title("Checkbutton")

        self.pack(fill=BOTH, expand=True)
        self.var = BooleanVar()

        w = 1000
        h = 625

        sw = self.parent.winfo_screenwidth()
        sh = self.parent.winfo_screenheight()

        x = (sw - w)/2
        y = (sh - h)/2
        self.parent.geometry('%dx%d+%d+%d' % (w, h, x, y))


        app1 = Button(self, text="Application 1", command=self.app1Click)
        app2 = Button(self, text="Application 2", command=self.app2Click)
        app3 = Button(self, text="Application 3", command=self.app3Click)
        app1.place(x=150, y=200)
        app2.place(x=300, y=200)
        app3.place(x=450, y=200)


    def onClick(self):

        if self.var.get() == True:
            self.master.title("Checkbutton")
        else:
            self.master.title("")

    def app1Click(self):
        print("application1")
        dir_path = os.path.dirname(os.path.realpath(__file__))
        dir_str_os = str(dir_path)
        dir_str = dir_str_os + "/Kayak"
        print(os.path.isdir(dir_str))
        print(dir_path)

        if(os.path.isdir(dir_str) == False):
            print("falsaasdfe")
            git.Git().clone("git://github.com/dschanoeh/Kayak")
            print("successfully cloned!")

        if(os.path.isdir(dir_str) == True):
            print("We already have this repository!")


    def app2Click(self):
        print("application2")
        dir_path = os.path.dirname(os.path.realpath(__file__))
        dir_str_os = str(dir_path)
        dir_str = dir_str_os + "/caringcaribou"
        print(os.path.isdir(dir_str))

        if(os.path.isdir(dir_str) == False):
            print("Need to install the application")
            git.Git().clone("https://github.com/CaringCaribou/caringcaribou.git")
            print("Successfully cloned!")
        
        if(os.path.isdir(dir_str) == False):
            print("We already have this repository")



    def app3Click(self):
        print("application3")


def main():

    root = Tk()
    root.geometry("250x150+300+300")
    app = Example(root)
    root.mainloop()


if __name__ == '__main__':
    main()