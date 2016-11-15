'''
from tkinter import Tk, BOTH, Label
from tkinter.ttk import Frame, Button, Style
#from PIL import Image, ImageTk


class Example(Frame):

    def __init__(self, parent):
        Frame.__init__(self, parent)

        self.parent = parent


        self.parent.title("Quit button")
        self.style = Style()
        self.style.theme_use("default")

        self.pack(fill=BOTH, expand=1)


        w = 1000
        h = 625

        sw = self.parent.winfo_screenwidth()
        sh = self.parent.winfo_screenheight()

        x = (sw - w)/2
        y = (sh - h)/2
        self.parent.geometry('%dx%d+%d+%d' % (w, h, x, y))

        quitButton = Button(self, text="Quit",
            command=self.quit)
        quitButton.place(x=900, y=550)


        def application1Click(self):
            print("Hello")


        def application2Click(self):
            print("Hello, Again!")

        def application3Click(self):
            print("Hello, for the third time!")


        app1 = Button(self, text="Application 1", command=self.application1Click)
        app2 = Button(self, command=self.application2Click, text="Application 2")
        app3 = Button(self, command=self.application3Click, text="Application 3")
        app1.place(x=150, y=200)
        app2.place(x=300, y=200)
        app3.place(x=450, y=200)




def main():

    root = Tk()
    root.geometry("250x150+300+300")
    app = Example(root)
    root.mainloop()


if __name__ == '__main__':
    main()


            w = 1000
        h = 625

        sw = self.parent.winfo_screenwidth()
        sh = self.parent.winfo_screenheight()

        x = (sw - w)/2
        y = (sh - h)/2
        self.parent.geometry('%dx%d+%d+%d' % (w, h, x, y))

        '''


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
        dir_str = str(dir_path)
        print(os.path.isdir(dir_str + "/Kayak"))
        print(dir_path)

        if(os.path.isdir(dir_str + "/Kayak") == False):
            print("falsaasdfe")
            git.Git().clone("git://github.com/dschanoeh/Kayak")
            print("successfully cloned!")

        if(os.path.isdir(dir_str + "/Kayak") == True):
            print("We already have this repository!")


    def app2Click(self):
        print("application2")

    def app3Click(self):
        print("application3")


def main():

    root = Tk()
    root.geometry("250x150+300+300")
    app = Example(root)
    root.mainloop()


if __name__ == '__main__':
    main()