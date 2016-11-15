'''#from tkinter import *
import tkinter as tk

class passwordDialog(tk.Toplevel):
    def __init__(self):
        tk.Toplevel.__init__(self)
        self.password = None
        self.entry = tk.Entry(self, show='*')
        self.entry.pack()
        self.button = tk.Button(self)
'''

import tkinter as tk
import tkinter.messagebox as tm

class application(tk.Tk):
    def __init__(self, *args, **kwargs):
        tk.__init__(self, *args, **kwargs)



    # We're making a container on which we're stacking a bunch of frames
    # The frame we want to see will be visible on the top

        container = tk.Frame(self)
        container.pack(side="top", fill="both", expand=True)
        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}
        for F in (LoginFrame, mainPage):
            page_name = F.__name__
            frame = F(parent=container, controller=self)
            self.frames[page_name] = frame

            ## put all pages in one location
            # the one on the top of the stacking order
            # will be the one that is visible

            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame("LoginFrame")


        def show_frame(self, pageName):
            # show a frame for page with given name

            frame = self.frames[page_name]
            frame.tkraise()



class loginCredentials():
    def __init__(self, name, pwd, auth):
        self.username = name
        self.password = pwd
        self.authenticator = auth

    username = ''
    password =''

    authenticator = False



class LoginFrame(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        tk.Frame.__init__(self, parent)
        self.controller = controller


        self.label_1 = tk.Label(self, text="Username")
        self.label_2 = tk.Label(self, text="Password")

        self.entry_1 = tk.Entry(self)
        self.entry_2 = tk.Entry(self, show="*")

        self.label_1.grid(row=0, sticky=tk.E)
        self.label_2.grid(row=1, sticky=tk.E)
        self.entry_1.grid(row=0, column=1)
        self.entry_2.grid(row=1, column=1)

        self.checkbox = tk.Checkbutton(self, text="Keep me logged in")
        self.checkbox.grid(columnspan=2)

        self.logbtn = tk.Button(self, text="Login", command = self._login_btn_clickked)
        self.logbtn.grid(columnspan=2)

        self.pack()

    def _login_btn_clickked(self, controller):
        #print("Clicked")
        username = self.entry_1.get()
        password = self.entry_2.get()

        #print(username, password)

        if username == "petar" and password == "autopen":
            tm.showinfo("Login info", "Welcome Petar")
            usr1 = loginCredentials(username, password, True)
            print(usr1.username)
            print(usr1.password)
            controller.showFrame("mainPage")

        else:
            tm.showerror("Login error", "Incorrect username")

    def _enter_btn_pressed(self):
        username = self.entry_1.get()
        password = self.entry_2.get()


class mainPage(tk.Frame):
    def __init__(self, parent, controller):
        tk.Frame.__init__(self, parent)
        self.controller = controller
        label = tk.Label(self, text="This is the second page")
        label.pack()



if __name__ == "__main__":
    app = application()
    app.mainloop()

