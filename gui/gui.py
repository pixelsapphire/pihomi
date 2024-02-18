from pathlib import Path
from tkinter import *
from mttkinter import mtTkinter
import tkinter.font as tkFont
from time import strftime
import time
from socket import *
import threading
import sys

OUTPUT_PATH = Path(__file__).parent
ASSETS_PATH = OUTPUT_PATH / Path(r"assets")

# NETWORK CONNECTION
HOST = 'localhost'
PORT = 3141
BUFF_SIZE = 64
try:
    with open('serverhost.txt', 'r') as host_file:
        HOST = host_file.readlines()[0].rstrip()
except:
    print('Could not get server host from serverhost.txt; defaulting to "localhost"...')
ADDRESS = (HOST, PORT)

# INTERFACE COLORS
dark = '#121212'
semi_dark = '#1e1e1e'
semi_semi_dark = '#2e2e2e'
purple = '#bb86fc'
light = '#bfbfbf'
semi_light = '#a8a8a8'
semi_semi_light = '#595959'
blue = '#397791'

# INTERFACE CONSTANTS
padding = 25
devices_titles = ["CLOCK", "IRRIGATION", "OUTLETS"]

# SYSTEM VARIABLES
active_devices = [0, 0, 0]
active_outlets = [0, 0, 0, 0]
water_level = 5
water_parameters = [0, 0]  # [frequency of watering, water volume]
water_frequency_titles = [['4 times a day', 0.25], ['2 times a day', 0.5], ['1 time a day', 1], ['every 2 days', 2],
                          ['every 4 days', 4], ['every week', 7], ['every 2 weeks', 14]]
water_volumes = [20, 50, 100, 150, 200, 250, 500]
parameter_change = 'x'  # string (w formacie 'x1') przechowuje żądanie zmiany wartości parametru (np. gdy włączamy gniazdko 4 to parameter_change = 's4'


def relative_to_assets(path: str) -> Path:
    return ASSETS_PATH / Path(path)


def make_gui():
    main_canvas = None

    def on_closing():
        stop_event.set()
        window.destroy()
        sys.exit()

    def draw_rectangle(canvas, x1, y1, x2, y2, color, fillcolor):
        return canvas.create_rectangle(
            x1, y1,
            x2, y2,
            fill=color,
            activefill=fillcolor,
            outline=""
        )

    def draw_text(canvas, x, y, text, color, size):
        return canvas.create_text(
            x, y,
            anchor="nw",
            text=text,
            fill=color,
            font=tkFont.Font(family='Inter', size=size, weight='bold')
        )

    def draw_image_array(canvas, arr, file, x, y):
        arr.append(PhotoImage(
            file=relative_to_assets(file)))
        return canvas.create_image(
            x, y,
            image=arr[-1]
        )

    # LOGO
    def draw_logo(image):
        logo_canvas = Canvas(main_canvas, bg=purple, height=80, width=950, bd=0, highlightthickness=0, relief="ridge")
        logo_canvas.create_image(60, 40, image=image)
        draw_text(logo_canvas, 100, 25, text="PiHoMi", color="#000000", size=30)
        logo_canvas.place(x=0, y=0)

    # DEVICES
    def draw_devices(parray_image_icon, parray_image_checkbox):
        sizes = [300, 400]
        coords = [50, 150]
        height_device_place = 60
        gap_after_text = 110
        icon_image_files = ["clock-light25x25.png", "water-light30x30.png", "socket-light-light-bcg30x30.png"]
        icon_image_disabled_files = ["clock-dark25x25.png", "water-dark30x30.png", "socket-dark-light-bcg30x30.png"]

        devices_canvas = Canvas(main_canvas, bg=semi_dark, height=sizes[1], width=sizes[0], bd=0, highlightthickness=0, relief="ridge")
        devices_canvas.place(x=coords[0], y=coords[1])

        draw_text(devices_canvas, padding + 10, padding + 10, text="Devices", color=light, size=30)

        for i in range(3):
            def on_button_click(event, index=i):
                global parameter_change
                parameter_change = "{device}".format(device=devices_titles[index][0].lower())

            container = draw_rectangle(
                devices_canvas,
                padding, gap_after_text + i * height_device_place + i * padding,
                         sizes[0] - padding, + gap_after_text + (i + 1) * height_device_place + i * padding,
                semi_semi_dark, dark
            )

            draw_image_array(
                devices_canvas,
                parray_image_icon,
                icon_image_files[i] if active_devices[i] == 1 else icon_image_disabled_files[i],
                padding + 30, gap_after_text + i * height_device_place + i * padding + 30
            )
            text = draw_text(
                devices_canvas,
                padding + 62, i * height_device_place + i * padding + gap_after_text + 23,
                text=devices_titles[i], color=semi_light if active_devices[i] == 1 else semi_semi_light, size=13
            )

            draw_image_array(
                devices_canvas,
                parray_image_checkbox,
                "checkbox-yes15x15.png" if active_devices[i] == 1 else "checkbox-no15x15.png",
                padding + 225,
                gap_after_text + i * height_device_place + i * padding + 30
            )

            devices_canvas.tag_bind(container, '<Button-1>', on_button_click)
            devices_canvas.tag_bind(text, '<Button-1>', on_button_click)

    # OUTLETS
    def draw_outlets(pactive_outlets, parray_image_outlet):
        outlets_height = 200
        sizes = [window_size[0] - (350 + 2 * padding + 50), outlets_height]
        coords = [350 + 2 * padding, 150]

        outlets_canvas = Canvas(main_canvas, bg=semi_dark, height=sizes[1], width=sizes[0], bd=0, highlightthickness=0, relief="ridge")
        outlets_canvas.place(x=coords[0], y=coords[1])

        draw_text(outlets_canvas, padding * 1.5, padding, text="Outlets", color=light, size=30)

        for i in range(4):
            def on_button_click(event, index=i):
                global parameter_change
                if active_devices[2]:
                    parameter_change = "o{outlet_num}".format(outlet_num=index + 1)

            image = draw_image_array(
                outlets_canvas,
                parray_image_outlet,
                "socket-light75x75.png" if pactive_outlets[i] == 1 else "socket-dark75x75.png",
                i * (padding - 10) + i * 90 + 95, 4 * padding + 20,
            )
            text = draw_text(
                outlets_canvas,
                i * (padding - 10) + i * 90 + 62, 4 * padding + 55,
                text="{i}. enabled".format(i=i + 1) if pactive_outlets[i] == 1 else "{i}. disabled".format(i=i + 1),
                color=semi_light if pactive_outlets[i] == 1 else semi_semi_light,
                size=10
            )
            outlets_canvas.tag_bind(image, '<Button-1>', on_button_click)
            outlets_canvas.tag_bind(text, '<Button-1>', on_button_click)

        return outlets_height

    # IRRIGATION
    def draw_irrigation(outlets_height, pwater_level=1):
        irrigation_width = 200
        sizes = [irrigation_width, outlets_height]
        coords = [350 + 2 * padding, 150 + outlets_height + padding]

        irrigation_canvas = Canvas(main_canvas, bg=semi_dark, height=sizes[1], width=sizes[0], bd=0, highlightthickness=0, relief="ridge")
        irrigation_canvas.place(x=coords[0], y=coords[1])

        text1 = draw_text(
            irrigation_canvas,
            30, 20,
            text="Water level", color=semi_light, size=15
        )
        text2 = draw_text(
            irrigation_canvas,
            30, 40,
            text="irrigation", color=semi_semi_light, size=10
        )

        # container rectangle
        draw_rectangle(
            irrigation_canvas,
            3 * padding, 3 * padding,
            3 * padding + 50, 3 * padding + 75,
            semi_semi_dark, semi_semi_dark
        )
        # water level rectangle
        draw_rectangle(
            irrigation_canvas,
            3 * padding, 3 * padding + (7 - pwater_level) * 10,
            3 * padding + 50, 3 * padding + 75,
            blue, blue
        )

        def change_frequency(freq):
            global water_parameters, parameter_change
            water_parameters[0] = freq
            parameter_change = "f{frequency_value}".format(frequency_value=freq)

        def change_volume(v):
            global water_parameters, parameter_change
            water_parameters[1] = v
            parameter_change = "v{volume_value}".format(volume_value=v)

        def popup(e):
            menu.tk_popup(e.x_root, e.y_root)

        menu = Menu(window, tearoff=False)
        menu.add_command(label="Irrigation frequency", state="disabled", activebackground=menu.cget("background"))
        menu.add_separator()
        for frequency in water_frequency_titles:
            menu.add_command(label=str(frequency[0]), command=lambda freq=frequency[1]: change_frequency(freq))
        menu.add_separator()

        menu.add_command(label="Water volume", state="disabled", activebackground=menu.cget("background"))
        menu.add_separator()
        for volume in water_volumes:
            menu.add_command(label=str(volume) + 'ml', command=lambda freq=volume: change_volume(freq))

        irrigation_canvas.bind("<Button-3>", popup)

        return irrigation_width

    # CLOCK
    def draw_clock(irrigation_width, outlets_height):
        sizes = [window_size[0] - (350 + 2 * padding + 50) - irrigation_width - padding, outlets_height]
        coords = [350 + 3 * padding + irrigation_width, 150 + outlets_height + padding]

        clock_canvas = Canvas(main_canvas, bg=semi_dark, height=sizes[1], width=sizes[0], bd=0, highlightthickness=0, relief="ridge")
        clock_canvas.place(x=coords[0], y=coords[1])

        draw_text(
            clock_canvas,
            padding, 20,
            text="Clock", color=light, size=30
        )

        def draw_time():
            string = strftime('%H:%M')
            lbl.config(text=string)
            lbl.after(1000, draw_time)

        lbl = Label(clock_canvas, font=('calibri', 40, 'bold'), background=semi_dark, foreground=semi_light)
        lbl.place(x=3 * padding, y=2 * padding + 20)
        draw_time()

    def draw_dashboard():
        global active_outlets, water_level, array_image_icon, array_image_checkbox, array_image_outlet

        array_image_icon = []
        array_image_checkbox = []
        array_image_outlet = []

        draw_devices(array_image_icon, array_image_checkbox)
        outlets_height = draw_outlets(active_outlets, array_image_outlet)
        irrigation_width = draw_irrigation(outlets_height, water_level)
        draw_clock(irrigation_width, outlets_height)

    def server_connection():
        global active_devices, active_outlets, water_level, water_parameters, parameter_change

        def popup_no_connection():
            popup = Toplevel(window)
            root_x = window.winfo_rootx()
            root_y = window.winfo_rooty()
            win_x = root_x + 325
            win_y = root_y + 175

            popup.geometry("300x250"f'+{win_x}+{win_y}')
            popup.title("No server connection")
            popup.protocol("WM_DELETE_WINDOW", on_closing)  # closing the window stops a thread for GUI
            label1 = Label(popup, text="Failed ", font=('calibri bold', 37), fg='red')
            label2 = Label(popup, text="to connect to server \n :<", font=('calibri', 15), fg='black')
            label1.place(relx = 0.52, rely = 0.35, anchor = CENTER)
            label2.place(relx = 0.5, rely = 0.6, anchor = CENTER)

        def update_global_params(data):
            global active_devices, active_outlets, water_level, water_parameters, parameter_change
            for i_device in range(len(active_devices)):
                active_devices[i_device] = int(data[i_device])
            for i_outlet in range(len(active_devices), len(active_devices) + len(active_outlets)):
                active_outlets[i_outlet - len(active_devices)] = int(data[i_outlet])
            water_level = int(data[len(active_outlets) + len(active_devices)])
            water_parameters[0] = float(data[-2])  # frequency of watering
            water_parameters[1] = int(data[-1])  # volume of water

        try:
            server: socket = socket(AF_INET, SOCK_STREAM)
            server.settimeout(3)
            server.connect(ADDRESS)

        except Exception as e:
            print(f"Error: {e}")
            # thread_socket.join()
            stop_event.set()
            popup_no_connection()
            exit(1)

        send_to_check_updates = 'x' + '\n'
        server.send(send_to_check_updates.encode('utf-8'))
        received_from_request = server.recv(BUFF_SIZE)
        while not stop_event.is_set():
            try:
                # update changes made by user to server
                if len(parameter_change) != 0:
                    data_to_send = parameter_change + '\n'
                    server.send(data_to_send.encode('utf-8'))
                    parameter_change = ''

                    received_data = server.recv(BUFF_SIZE)
                    parts = received_data.decode('utf-8').split(';')
                    update_global_params(parts)

                    if main_canvas is not None:
                        draw_dashboard()
                    time.sleep(1)

                send_to_check_updates = 'x' + '\n'
                server.send(send_to_check_updates.encode('utf-8'))
                current_received = server.recv(BUFF_SIZE)
                if current_received != received_from_request:
                    parts = current_received.decode('utf-8').split(';')

                    update_global_params(parts)
                    draw_dashboard()
                    received_from_request = current_received
                time.sleep(1)

            except Exception as e:
                print(f"Error: {e}")
                # thread_socket.join()
                stop_event.set()
                exit(1)

    stop_event = threading.Event()

    thread_socket = threading.Thread(target=server_connection)
    thread_socket.start()

    window = mtTkinter.Tk()
    window.protocol("WM_DELETE_WINDOW", on_closing)  # closing the window stops a thread for GUI
    window.title('PiHoMi')
    window.geometry("950x600")
    window.resizable(False, False)
    window.configure(bg=dark)
    window_size = [950, 600]

    main_canvas = Canvas(
        window,
        bg=dark,
        height=550,
        width=1000,
        bd=0,
        highlightthickness=0,
        relief="ridge"
    )
    main_canvas.pack()

    image_home = PhotoImage(file=relative_to_assets("home.png"))
    draw_logo(image_home)

    draw_dashboard()

    window.mainloop()


try:
    make_gui()
except KeyboardInterrupt:
    sys.exit(0)