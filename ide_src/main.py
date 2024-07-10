import shutil
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
import tkinter.font as tkfont
import subprocess
import threading
from tkinter import ttk
import pickle
import os
import sys

SOURCE_EXTENSION = ".ant"
SOURCE_EXTENSION_REGEX = f"*{SOURCE_EXTENSION}"
C_ACC_LIBRARY_EXTENSION_REGEX = "*.c"

SOURCE_DESCRIPTION = "AnteaterLang Source"
C_ACC_LIBRARY_EXTENSION_DESCRIPTION = "C Acc Library Source"

TERMINAL_BACKGROUND_COLOR = '#2B2B2B'
EDITOR_BACKGROUND_COLOR = '#2B2B2B'

TERMINAL_FONT_SIZE = 15
EDITOR_FONT_SIZE = 15
TAB_SPACE = 4

IDE_PROJECT_SETTING_NAME = "/ideconfig.pickle"

DEFAULT_WINDOW_SIZE = "1600x800"

ANTEATERLANG_KEYWORD = "antlang"

C_ACC_LIBRARY_SO_PATH = "libuserFunctions.so"

IDE_VERSION = "0.1.0"

SAMPLE_ANT_SOURCE = """\
void function native_hello_world() {
    println("Hello World!");
}

void function main() {

    # Builtin function
    native_hello_world();

    # C function
    c_hello_world();
}
"""

SAMPLE_C_ACC_LIBRARY_SOURCE = """\
#include "chunk.h"
#include "object.h"
#include "primitiveVars.h"

// Use this macro to define a new user function in the array
#define USER_FUNCTION(inCount, outCount, funcName, cFunction) {.in = inCount, .out = outCount, .name = funcName, .cFunc = cFunction}

// This is the definition of a user function
// Value (*cMethodType)(Value, Value*, int);

// Define functions here

// Sample Function
Value helloWorld(Value self, Value* args, int numArgs){
    printf("Hello World!\n");

    return NONE_VAL;
}


// This is what the VM will use to integrate with the host language

// Change the number of functions here
uint32_t funcCount = 1;

// Add the functions here, add a new line & comma for each function
userFunction userFuncs[] = {
USER_FUNCTION(0, 0, "c_hello_world", helloWorld)
};
"""

c_keywords = {
    "auto": "#CC7832",
    "break": "#CC7832",
    "case": "#CC7832",
    "char": "#CC7832",
    "const": "#CC7832",
    "continue": "#CC7832",
    "default": "#CC7832",
    "do": "#CC7832",
    "double": "#CC7832",
    "else": "#94558D",
    "enum": "#CC7832",
    "extern": "#CC7832",
    "float": "#CC7832",
    "for": "#A74926",
    "goto": "#CC7832",
    "if": "#94558D",
    "inline": "#CC7832",
    "int": "#CC7832",
    "long": "#CC7832",
    "register": "#CC7832",
    "restrict": "#CC7832",
    "return": "#688558",
    "short": "#CC7832",
    "signed": "#CC7832",
    "sizeof": "#CC7832",
    "static": "#CC7832",
    "struct": "#C47432",
    "switch": "#CC7832",
    "typedef": "#CC7832",
    "union": "#CC7832",
    "unsigned": "#CC7832",
    "void": "#688558",
    "volatile": "#CC7832",
    "while": "#A74926",
    "include": "#A74926",
}

ant_keywords = {
    "if": "#94558D",  # (148, 85, 141)
    "elif": "#94558D",  # (148, 85, 141)
    "else": "#94558D",  # (148, 85, 141)
    "while": "#A74926",  # (167, 73, 38)
    "for": "#A74926",  # (167, 73, 38)
    "break": "#CC7832",  # (204, 120, 50)
    "continue": "#CC7832",  # (204, 120, 50)
    "return": "#688558",  # (104, 133, 88)
    "void": "#688558",  # (104, 133, 88)
    "true": "#6897BB",  # (104, 151, 187)
    "false": "#6897BB",  # (104, 151, 187)
    "is": "#8888C6",  # (136, 136, 198)
    "none": "#8888C6",  # (136, 136, 198)
    "class": "#C47432",  # (196, 116, 50)
    "function": "#C47432",  # (196, 116, 50)
    "self": "#94558D",  # (148, 85, 141)
    "new": "#A74926",  # (167, 73, 38)
    "pInit": "#CC7832",  # (204, 120, 50)
    "init": "#688558",  # (104, 133, 88)
    "global": "#6897BB",  # (104, 151, 187)
    "or": "#8888C6",  # (136, 136, 198)
    "and": "#C47432",  # (196, 116, 50)
    "not": "#94558D",  # (148, 85, 141)
    "include": "#A74926",  # (167, 73, 38)
    "exception": "#CC7832",  # (204, 120, 50)
    "unrecoverable": "#688558",  # (104, 133, 88)
    "handle": "#6897BB",  # (104, 151, 187)
    "try": "#8888C6",  # (136, 136, 198)
    "raise": "#C47432",  # (196, 116, 50)
}

class AnteaterIDE:
    def __init__(self, root):
        self.root = root
        self.root.title("Anteater IDE")
        self.root.geometry(DEFAULT_WINDOW_SIZE)

        self.editor_font_size = EDITOR_FONT_SIZE  # Default font size
        self.terminal_font_size = TERMINAL_FONT_SIZE  # Default terminal font size
        self.tab_spaces = TAB_SPACE  # Number of spaces per tab

        # Set up the main frame
        self.main_frame = tk.Frame(self.root)
        self.main_frame.pack(fill=tk.BOTH, expand=1)

        # Create a PanedWindow with horizontal orientation
        self.paned_window = tk.PanedWindow(self.main_frame, orient=tk.HORIZONTAL, sashwidth=10)
        self.paned_window.pack(fill=tk.BOTH, expand=1)

        # Create a frame for the text editor
        self.editor_frame = tk.Frame(self.paned_window, width=800)
        self.editor_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=1)
        self.editor_frame.pack_propagate(False)

        # Create a frame for the terminal
        self.terminal_frame = tk.Frame(self.paned_window, width=400)
        self.terminal_frame.pack(side=tk.RIGHT, fill=tk.BOTH)
        self.terminal_frame.pack_propagate(False)

        # Add the frames to the PanedWindow
        self.paned_window.add(self.editor_frame, stretch="always")
        self.paned_window.add(self.terminal_frame, stretch="always")

        # Create a Notebook widget for multiple tabs
        self.notebook = ttk.Notebook(self.editor_frame)
        self.notebook.pack(fill=tk.BOTH, expand=1)

        # Create a Text widget for the terminal output
        self.terminal_output = tk.Text(self.terminal_frame, height=30, bg=TERMINAL_BACKGROUND_COLOR, fg='white', font=("Courier", self.terminal_font_size), wrap=tk.NONE)
        self.terminal_output.pack(side=tk.TOP, fill=tk.BOTH, expand=1)

        # Create a Scrollbar for the terminal output
        self.terminal_scrollbar = tk.Scrollbar(self.terminal_output)
        self.terminal_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.terminal_output.config(yscrollcommand=self.terminal_scrollbar.set)
        self.terminal_scrollbar.config(command=self.terminal_output.yview)

        # Terminal input
        self.terminal_input = tk.Entry(self.terminal_frame, bg='black', fg='white')
        self.terminal_input.pack(side=tk.BOTTOM, fill=tk.X)
        self.terminal_input.bind("<Return>", self.run_terminal_command)

        # Create menu bar
        self.menu_bar = tk.Menu(self.root)
        self.root.config(menu=self.menu_bar)

        # File menu
        file_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.menu_bar.add_cascade(label="File", menu=file_menu)
        file_menu.add_separator()  # Add a separator for better organization
        file_menu.add_command(label="New File", command=self.new_file)
        file_menu.add_command(label="Open File", command=self.open_file)
        file_menu.add_command(label="Save File", command=self.save_file)
        file_menu.add_command(label="Save All Files", command=self.save_all_files)
        file_menu.add_command(label="Save As", command=self.save_as_file)

        file_menu.add_separator()
        file_menu.add_command(label="Close Tab", command=self.close_current_tab)

        # Edit menu
        edit_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.menu_bar.add_cascade(label="Edit", menu=edit_menu)
        edit_menu.add_command(label="Undo", command=self.undo)
        edit_menu.add_command(label="Redo", command=self.redo)
        edit_menu.add_separator()
        edit_menu.add_command(label="Cut", command=self.cut)
        edit_menu.add_command(label="Copy", command=self.copy)
        edit_menu.add_command(label="Paste", command=self.paste)
        edit_menu.add_command(label="Delete", command=self.delete)
        edit_menu.add_separator()
        edit_menu.add_command(label="Select All", command=self.select_all)
        edit_menu.add_separator()
        edit_menu.add_command(label="Change Editor Font Size", command=self.change_editor_font_size)
        edit_menu.add_command(label="Change Terminal Font Size", command=self.change_terminal_font_size)

        # Run menu
        run_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.menu_bar.add_cascade(label="Run", menu=run_menu)

        # Add menu items for setting Anteater Lang Source and C Acc Library Source
        run_menu.add_command(label="Set AnteaterLang Source", command=self.set_anteater_lang_source)
        run_menu.add_command(label="Set C Acc Library Source", command=self.set_c_acc_library_source)
        run_menu.add_separator()
        run_menu.add_command(label="Run", command=self.run_code)
        run_menu.add_command(label="Run Without C Lib", command=self.run_code_without_c_lib)
        run_menu.add_command(label="Compile C Lib", command=self.compile_c_lib)

        # AnteaterLang menu
        anteater_lang_menu = tk.Menu(self.menu_bar, tearoff=0)

        self.menu_bar.add_cascade(label="AnteaterLang", menu=anteater_lang_menu)
        anteater_lang_menu.add_command(label="Check Language Version", command=self.check_language_info)
        anteater_lang_menu.add_command(label="Check IDE Version", command=self.check_ide_info)

        # Add divider
        anteater_lang_menu.add_separator()

        # Add default source code
        anteater_lang_menu.add_command(label="Load Sample AnteaterLang Source", command=self.load_ant_default_source)
        anteater_lang_menu.add_command(label="Load Sample C Acc Library Source", command=self.load_c_acc_default_source)

        # Set up the buttons frame
        self.buttons_frame = tk.Frame(self.root)
        self.buttons_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=5, pady=5)

        # Create buttons
        compile_c_lib_button = tk.Button(self.buttons_frame, text="Compile C Lib", command=self.compile_c_lib)
        compile_c_lib_button.pack(side=tk.LEFT, padx=5)

        run_button = tk.Button(self.buttons_frame, text="Run", command=self.run_code)
        run_button.pack(side=tk.LEFT, padx=5)

        run_without_c_lib_button = tk.Button(self.buttons_frame, text="Run w/o C Lib",
                                             command=self.run_code_without_c_lib)
        run_without_c_lib_button.pack(side=tk.LEFT, padx=5)

        # Bind keyboard shortcuts for Save, Open, and New
        self.root.bind_all("<Command-s>", lambda event: self.save_file())
        self.root.bind_all("<Command-o>", lambda event: self.open_file())
        self.root.bind_all("<Command-n>", lambda event: self.new_file())

        # Initialize variables to store file locations
        self.anteater_lang_source = None
        self.c_acc_library_source = None

        # Ask for project location
        self.ask_for_project_location()

        # Set home directory
        os.chdir(self.project_location)

        self.file_paths = dict()

        # Try to load the configuration
        self.load_config()

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

        self.language_version_info = self.check_language_version()

    def load_ant_default_source(self):
        self.new_file()
        # Past the sample source code
        self.get_current_text_widget().insert(tk.END, SAMPLE_ANT_SOURCE)

    def load_c_acc_default_source(self):
        self.new_file()
        # Past the sample source code
        self.get_current_text_widget().insert(tk.END, SAMPLE_C_ACC_LIBRARY_SOURCE)

    def check_language_info(self, event=None):
        self.check_language_version()
        if self.language_version_info:
            messagebox.showinfo("AnteaterLang Version", self.language_version_info)

    def check_ide_info(self, event=None):
        messagebox.showinfo("IDE Version", f"IDE Version: {IDE_VERSION}")

    def check_language_version(self) -> str:
        # Check if the AnteaterLang version is installed
        # Check the Python version and use the appropriate argument
        if sys.version_info >= (3, 7):
            process = subprocess.Popen(f"{ANTEATERLANG_KEYWORD} --version", shell=True, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       text=True)
        else:
            process = subprocess.Popen(f"{ANTEATERLANG_KEYWORD} --version", shell=True, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       universal_newlines=True)
        stdout, stderr = process.communicate()
        if not stdout.startswith('AnteaterLang Version'):
            messagebox.showinfo("AnteaterLang Version", "AnteaterLang is not installed/found.")
            return None
        return stdout

    def save_config(self):
        opened_file_local_paths = []
        for global_path in self.file_paths.values():
            opened_file_local_paths.append(global_path.split(self.project_location)[1])

        configs = {
            "project_location": self.project_location,
            "editor_font_size": self.editor_font_size,
            "terminal_font_size": self.terminal_font_size,
            "tab_spaces": self.tab_spaces,
            "anteater_lang_source": self.anteater_lang_source,
            "c_acc_library_source": self.c_acc_library_source,
            "files_opened": opened_file_local_paths
        }
        # Find setting location by joining base path and setting file name
        config_loc = self.project_location + IDE_PROJECT_SETTING_NAME
        with open(config_loc, "wb") as file:
            pickle.dump(configs, file)

    def new_tab(self, file_path):
        with open(file_path, "r") as f:
            new_tab = tk.Frame(self.notebook)

            # Create a Text widget for the editor with a vertical scrollbar
            text_widget = tk.Text(new_tab, undo=True, font=("Courier", self.editor_font_size),
                                  bg=EDITOR_BACKGROUND_COLOR, fg='white', wrap=tk.NONE)
            text_widget.pack(side=tk.LEFT, fill=tk.BOTH, expand=1)

            # Create a Scrollbar and attach it to the Text widget
            scrollbar = tk.Scrollbar(new_tab, orient=tk.VERTICAL, command=text_widget.yview)
            scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
            text_widget.config(yscrollcommand=scrollbar.set)

            text_widget.bind("<KeyRelease>", self.highlight_keywords)
            text_widget.bind("<Return>", self.auto_indent)

            file_content = f.read()
            text_widget.insert(tk.END, file_content)

            self.notebook.add(new_tab, text=file_path.split("/")[-1])
            self.notebook.select(new_tab)
            self.file_paths[str(new_tab)] = file_path
            self.highlight_keywords()

    def load_config(self):
        # Check if file exists
        config_loc = self.project_location + IDE_PROJECT_SETTING_NAME
        if not os.path.exists(config_loc):
            return
        try:
            with open(config_loc, "rb") as config_file:
                configs = pickle.load(config_file)
                # Set the window size
                self.project_location = configs["project_location"]
                self.editor_font_size = configs["editor_font_size"]
                self.terminal_font_size = configs["terminal_font_size"]
                self.tab_spaces = configs["tab_spaces"]
                self.anteater_lang_source = configs["anteater_lang_source"]
                self.c_acc_library_source = configs["c_acc_library_source"]
                for local_path in configs["files_opened"]:
                    file = self.project_location + "/" + local_path
                    self.new_tab(file)
        except:
            # Remove file and reset variables
            shutil.rmtree(config_loc)

            # Print message
            self.print_message(f"Prior IDE Cache @ {config_loc} is corrupted, file removed.")

            self.project_location = None
            self.editor_font_size = EDITOR_FONT_SIZE
            self.terminal_font_size = TERMINAL_FONT_SIZE
            self.tab_spaces = TAB_SPACE
            self.anteater_lang_source = None
            self.c_acc_library_source = None

            # Close all tabs
            for tab_id in self.notebook.tabs():
                self.notebook.forget(tab_id)



    def on_close(self):
        """Prompt the user to save all files before exiting."""
        if messagebox.askyesno("Exit", "Do you want to save all files before exiting?"):
            self.save_all_files()
        self.root.destroy()
        self.save_config()

    def print_message(self, message, color="blue"):
        color_map = {
            "red": "#FF0000",
            "green": "#00FF00",
            "blue": "#0000FF",
            "white": "#FFFFFF",
            "yellow": "#FFFF00",
            "cyan": "#00FFFF",
            "magenta": "#FF00FF",
            "black": "#000000",
        }

        # Ensure the color exists in the color_map, default to white if not found
        text_color = color_map.get(color, "#FFFFFF")

        # Insert the message into the terminal_output Text widget
        self.terminal_output.insert(tk.END, message + "\n", color)

        # Configure the tag to use the specified color
        self.terminal_output.tag_config(color, foreground=text_color)

        # Automatically scroll to the end of the terminal_output
        self.terminal_output.see(tk.END)

    def ask_for_project_location(self):
        self.project_location = filedialog.askdirectory(title="Select Project Location")
        if not self.project_location:
            messagebox.showinfo("Project Location", "No project location selected, defaulting to current directory.")
            self.project_location = "."

    def set_tab_size(self, spaces):
        font = tkfont.Font(font=self.get_current_text_widget()['font'])
        tab_width = font.measure(' ' * spaces)
        self.get_current_text_widget().config(tabs=(tab_width,))

    def new_file(self):
        file_path = filedialog.asksaveasfilename(initialdir=self.project_location, defaultextension=SOURCE_EXTENSION,
                                                 filetypes=[(SOURCE_DESCRIPTION, SOURCE_EXTENSION_REGEX), (C_ACC_LIBRARY_EXTENSION_DESCRIPTION, C_ACC_LIBRARY_EXTENSION_REGEX)])

        if file_path:
            # Create a file
            with open(file_path, "w", encoding='utf-8') as file:
                file.write("")
            self.new_tab(file_path)

    def open_file(self):
        file_paths = filedialog.askopenfilenames(initialdir=self.project_location, defaultextension=SOURCE_EXTENSION,
                                                 filetypes=[(SOURCE_DESCRIPTION, SOURCE_EXTENSION_REGEX), (
                                                 C_ACC_LIBRARY_EXTENSION_DESCRIPTION, C_ACC_LIBRARY_EXTENSION_REGEX)])
        for file_path in file_paths:
            self.new_tab(file_path)

    def save_file(self):
        current_tab = self.notebook.select()
        # Retrieve the file path stored in the tab's custom option
        file_path = self.file_paths[str(current_tab)]
        if file_path and not file_path.startswith("Untitled"):
            with open(file_path, "w", encoding='utf-8') as file:
                text = self.get_current_text_widget().get(1.0, tk.END)
                file.write(text)
        else:
            self.save_as_file()

    def save_all_files(self):
        for tab_id in self.notebook.tabs():
            file_path = self.file_paths.get(str(tab_id))
            if file_path:  # Check if the file path exists
                text_widget = self.notebook.nametowidget(tab_id).winfo_children()[0]
                content = text_widget.get("1.0", tk.END)
                with open(file_path, "w", encoding='utf-8') as file:
                    file.write(content)

    def save_as_file(self):
        file_path = filedialog.asksaveasfilename(initialdir=self.project_location, defaultextension=SOURCE_EXTENSION,
                                                 filetypes=[(SOURCE_DESCRIPTION, SOURCE_EXTENSION_REGEX), (
                                                 C_ACC_LIBRARY_EXTENSION_DESCRIPTION, C_ACC_LIBRARY_EXTENSION_REGEX)])
        if file_path:
            with open(file_path, "w", encoding='utf-8') as file:
                file.write(self.get_current_text_widget().get(1.0, tk.END))
            current_tab = self.notebook.select()
            self.notebook.tab(current_tab, text=file_path.split("/")[-1])
            self.notebook.tab(current_tab, text=file_path)  # Update the tab's text to the new file path

    def undo(self):
        self.get_current_text_widget().event_generate("<<Undo>>")

    def redo(self):
        self.get_current_text_widget().event_generate("<<Redo>>")

    def cut(self):
        self.get_current_text_widget().event_generate("<<Cut>>")

    def copy(self):
        self.get_current_text_widget().event_generate("<<Copy>>")

    def paste(self):
        self.get_current_text_widget().event_generate("<<Paste>>")

    def delete(self):
        self.get_current_text_widget().delete(tk.SEL_FIRST, tk.SEL_LAST)

    def select_all(self):
        self.get_current_text_widget().tag_add(tk.SEL, "1.0", tk.END)
        self.get_current_text_widget().mark_set(tk.INSERT, "1.0")
        self.get_current_text_widget().see(tk.INSERT)

    def change_editor_font_size(self):
        size = simpledialog.askinteger("Font Size", "Enter new font size:", initialvalue=self.editor_font_size)
        if size:
            self.editor_font_size = size
            self.get_current_text_widget().config(font=("Courier", self.editor_font_size))
            self.set_tab_size(self.tab_spaces)  # Update tab size with new font size

    def change_terminal_font_size(self):
        size = simpledialog.askinteger("Font Size", "Enter new font size:", initialvalue=self.terminal_font_size)
        if size:
            self.terminal_font_size = size
            self.terminal_output.config(font=("Courier", self.terminal_font_size))

    def highlight_keywords(self, event=None):
        '''Apply syntax highlighting to the text based on defined keywords and their associated colors.'''
        text_widget = self.get_current_text_widget()

        current_tab = self.notebook.select()
        file_path = self.file_paths[str(current_tab)]

        # Determine the file type and select the appropriate keyword set
        if file_path.endswith('.c'):
            keyword_colors = c_keywords
        elif file_path.endswith('.ant'):
            keyword_colors = ant_keywords
        else:
            return  # No highlighting for unsupported file types

        # Clear all previous highlights
        text_widget.tag_remove("highlight", "1.0", "end")

        # Start from the beginning of the text area
        start_idx = "1.0"
        while True:
            # Find the next occurrence of any keyword
            match = text_widget.search(r'\m(' + '|'.join(keyword_colors.keys()) + r')\M', start_idx, stopindex="end",
                                       regexp=True)
            if not match:
                break  # No more keywords found, exit loop

            # Get the matched keyword to determine the appropriate color
            matched_keyword = text_widget.get(match, f"{match} wordend").strip()
            match_length = len(matched_keyword)
            end_idx = f"{match}+{match_length}c"

            # Highlight the matched keyword with its associated color
            text_widget.tag_add(matched_keyword, match, end_idx)
            text_widget.tag_config(matched_keyword, foreground=keyword_colors[matched_keyword])

            # Move start index past the current match to continue searching
            start_idx = end_idx

    def auto_indent(self, event):
        text_widget = self.get_current_text_widget()
        current_line_index = text_widget.index(tk.INSERT).split(".")[0]
        current_line = text_widget.get(f"{current_line_index}.0", f"{current_line_index}.end")

        indent_level = len(current_line) - len(current_line.lstrip())
        extra_indent = 0

        # Get the current cursor position
        cursor_pos = self.get_current_text_widget().index(tk.INSERT)
        # Calculate the position of the character before the cursor
        pos_before_cursor = f"{cursor_pos} - 1 char"
        # Get the character before the cursor
        char_before_cursor = self.get_current_text_widget().get(pos_before_cursor, cursor_pos)

        new_brace = char_before_cursor == "{"

        if new_brace:
            extra_indent = self.tab_spaces

        elif current_line.strip().endswith("}"):
            indent_level -= self.tab_spaces

        new_indent = ' ' * (indent_level + extra_indent)
        text_widget.insert(tk.INSERT, f"\n{new_indent}")

        if new_brace:
            text_widget.insert(tk.INSERT, "\n" + ' ' * indent_level + "}")
            # Set cursor position to the line between the braces
            text_widget.mark_set(tk.INSERT, f"{int(current_line_index)+1}.end")

        return "break"

    def close_current_tab(self):
        current_tab = self.notebook.select()
        if not current_tab:
            return  # No tab selected, nothing to close

        file_path = self.file_paths.get(str(current_tab))
        if file_path:
            # Prompt the user to save the file if it's associated with a tab
            if messagebox.askyesno("Save File", "Do you want to save changes to this file before closing?"):
                self.save_file()

        # Remove the tab from the notebook and its file path from the dictionary
        self.notebook.forget(current_tab)
        if file_path:
            del self.file_paths[str(current_tab)]

    def run_code(self):
        # Check that the AnteaterLang Source has been set
        if not self.anteater_lang_source:
            messagebox.showinfo("Source Not Set", f"Please set the AnteaterLang Source file.")
            return

        # Find the c acc file
        c_acc_so_path = os.path.join(self.project_location, C_ACC_LIBRARY_SO_PATH)

        # Check if the c acc file exists
        if not os.path.exists(c_acc_so_path):
            messagebox.showinfo("C Acc Library Not Found", "The C Acc Library has not been compiled. Please compile the C Acc Library before running the code.")
            return

        # Save everything before running
        self.save_all_files()

        # Clear the terminal output
        self.terminal_output.delete(1.0, tk.END)

        command = f"antlang {c_acc_so_path} {self.anteater_lang_source}"

        def run_command():
            if sys.version_info >= (3, 7):
                process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            else:
                process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            stdout, stderr = process.communicate()
            self.terminal_output.insert(tk.END, stdout)
            if stderr:
                self.terminal_output.insert(tk.END, stderr)
            self.terminal_output.insert(tk.END, "\n")

        threading.Thread(target=run_command).start()

    def run_code_without_c_lib(self):
        # Check that the AnteaterLang Source has been set
        if not self.anteater_lang_source:
            messagebox.showinfo("Source Not Set", f"Please set the AnteaterLang Source file.")
            return

        # Save everything before running
        self.save_all_files()

        # Clear the terminal output
        self.terminal_output.delete(1.0, tk.END)

        # Print source path
        self.print_message(f"Using AnteaterLang source @ {self.anteater_lang_source}")

        command = f"antlang {self.anteater_lang_source}"

        def run_command():
            if sys.version_info >= (3, 7):
                process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            else:
                process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            stdout, stderr = process.communicate()
            self.terminal_output.insert(tk.END, stdout)
            if stderr:
                self.terminal_output.insert(tk.END, stderr)
            self.terminal_output.insert(tk.END, "\n")

        threading.Thread(target=run_command).start()

    def compile_c_lib(self):
        # Check that the C Acc Library Source has been set
        if not self.c_acc_library_source:
            messagebox.showinfo("Source Not Set", f"Please set the C Acc Library Source file.")
            return
        # Save everything before running
        self.save_all_files()

        # Clear the terminal output
        self.terminal_output.delete(1.0, tk.END)

        include_dir = "/usr/local/include/anteaterlang"
        lib_dir = "/usr/local/lib"

        compile_command = [
            "gcc",
            "-fPIC",
            f"-I{include_dir}",
            "-c",
            self.c_acc_library_source,
            "-o",
            "userFunctions.o"
        ]

        shared_lib_command = [
            "gcc",
            "-shared",
            "-o",
            C_ACC_LIBRARY_SO_PATH,
            "userFunctions.o",
            f"-L{lib_dir}",
            "-llang"
        ]

        def compile_and_link():
            if sys.version_info >= (3, 7):
                compile_result = subprocess.run(" ".join(compile_command), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            else:
                compile_result = subprocess.run(" ".join(compile_command), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            self.terminal_output.insert(tk.END, compile_result.stdout)
            if compile_result.stderr:
                self.terminal_output.insert(tk.END, compile_result.stderr)
                return

            if compile_result.returncode == 0:
                if sys.version_info >= (3, 7):
                    shared_lib_result = subprocess.run(" ".join(shared_lib_command), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                else:
                    shared_lib_result = subprocess.run(" ".join(shared_lib_command), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
                self.terminal_output.insert(tk.END, shared_lib_result.stdout)
                if shared_lib_result.stderr:
                    self.terminal_output.insert(tk.END, shared_lib_result.stderr)

                # Print the success message
                self.print_message("C Acc Library compiled successfully!", "green")

                # Clean up
                if os.path.exists("userFunctions.o"):
                    os.remove("userFunctions.o")

        threading.Thread(target=compile_and_link).start()


    def set_anteater_lang_source(self):
        file_path = filedialog.askopenfilename(initialdir=self.project_location, title="Select AnteaterLang Source File",
                                               filetypes=[(SOURCE_DESCRIPTION, SOURCE_EXTENSION_REGEX)])
        if file_path:
            self.anteater_lang_source = file_path

    def set_c_acc_library_source(self):
        file_path = filedialog.askopenfilename(initialdir=self.project_location, title="Select C Acc Library Source File",
                                               filetypes=[(C_ACC_LIBRARY_EXTENSION_DESCRIPTION, C_ACC_LIBRARY_EXTENSION_REGEX)])
        if file_path:
            self.c_acc_library_source = file_path

    def run_terminal_command(self, event):
        command = self.terminal_input.get()
        self.terminal_input.delete(0, tk.END)
        self.terminal_output.insert(tk.END, f"$ {command}\n")

        def run_command():
            process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            stdout, stderr = process.communicate()
            self.terminal_output.insert(tk.END, stdout)
            if stderr:
                self.terminal_output.insert(tk.END, stderr)
            self.terminal_output.insert(tk.END, "\n")

        threading.Thread(target=run_command).start()

    def get_current_text_widget(self):
        current_tab = self.notebook.select()
        current_text_widget = self.notebook.nametowidget(current_tab).winfo_children()[0]
        return current_text_widget

if __name__ == "__main__":
    root = tk.Tk()
    editor = AnteaterIDE(root)
    root.mainloop()
