import os

def chk(label, path):
    print(label + ": " + ("OK" if os.path.exists(path) else "NOT FOUND"))

chk("Keil_v5 folder  ", "C:/Keil_v5")
chk("Keil UV4.exe    ", "C:/Keil_v5/UV4/UV4.exe")
chk("Keil ARM/PACK   ", "C:/Keil_v5/ARM/PACK")
chk("SONiX DFP Pack  ", "C:/Keil_v5/ARM/PACK/SONiX")
chk("Proteus (da go?)", "C:/Program Files (x86)/Labcenter Electronics/Proteus 8 Professional")

# Also check AppData pack location (Keil 5.37+ stores packs here)
chk("AppData SONiX   ", "C:/Users/MC HOME/AppData/Local/Arm/Packs/SONiX")
