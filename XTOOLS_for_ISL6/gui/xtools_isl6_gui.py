#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""XTOOLS ISL6 PC GUI V0.9 UNIVERSAL - CALIBRATION / EEPROM SERVICE. Requires: py -m pip install pyserial"""
import re, queue, threading, time
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from datetime import datetime
from pathlib import Path
import serial
from serial.tools import list_ports

DUMP_RE=re.compile(r"^\s*([0-9A-Fa-f]{2})\s+([0-9A-Fa-f]{4})\s*$")
READ_RE=re.compile(r"READ\s+0x([0-9A-Fa-f]{2})\s*=\s*0x([0-9A-Fa-f]{4})",re.I)
CAL={"CAL1":(1,9,14),"CAL2":(3,6,11),"CAL3":(4,7,12),"CAL4":(5,8,13)}
MEAN={0:"Unknown",1:"CAL1 primary",2:"Unknown",3:"CAL2 primary",4:"CAL3 primary",
5:"CAL4 primary",6:"CAL2 complement",7:"CAL3 complement",8:"CAL4 complement",
9:"CAL1 complement",10:"Serial number",11:"CAL2 complement",12:"CAL3 complement",
13:"CAL4 complement",14:"CAL1 complement",15:"Unknown"}

class App:
 def __init__(self,r):
  self.r=r;r.title("XTOOLS for Schulze ISL6 - Calibration & EEPROM Service Tool");r.geometry("1120x820")
  self.ser=None;self.stop=threading.Event();self.q=queue.Queue();self.buf=""
  self.cur=[None]*64;self.ref=[None]*64;self.refname="";self.indump=False
  self.hwmode="--";self.model="Not reported"
  self.ui();self.ports();r.after(40,self.poll);r.protocol("WM_DELETE_WINDOW",self.close)

 def ui(self):
  h=ttk.LabelFrame(self.r,text="XTOOLS for Schulze ISL6",padding=8);h.pack(fill="x",padx=8,pady=(8,6))
  ttk.Label(h,text="Calibration & EEPROM Service Tool",font=("TkDefaultFont",12,"bold")).pack(anchor="w")
  self.hwinfo=tk.StringVar(value="Hardware: waiting for INFO from XTOOLS")
  ttk.Label(h,textvariable=self.hwinfo).pack(anchor="w",pady=(4,0))

  f=ttk.Frame(self.r,padding=8);f.pack(fill="x")
  ttk.Label(f,text="COM:").pack(side="left");self.pv=tk.StringVar()
  self.pc=ttk.Combobox(f,textvariable=self.pv,width=12,state="readonly");self.pc.pack(side="left",padx=5)
  ttk.Button(f,text="Refresh",command=self.ports).pack(side="left")
  self.cb=ttk.Button(f,text="Connect",command=self.conn);self.cb.pack(side="left",padx=8)
  self.st=tk.StringVar(value="Disconnected");ttk.Label(f,textvariable=self.st).pack(side="left")

  c=ttk.LabelFrame(self.r,text="Read-only commands",padding=8);c.pack(fill="x",padx=8,pady=(0,8))
  for x in ("PING","INFO","DUMP"):
   ttk.Button(c,text=x if x!="DUMP" else "DUMP EEPROM",command=lambda z=x:self.send(z)).pack(side="left",padx=3)
  ttk.Label(c,text="  READ:").pack(side="left");self.av=tk.StringVar(value="00")
  ttk.Entry(c,textvariable=self.av,width=5).pack(side="left");ttk.Button(c,text="READ",command=self.readone).pack(side="left",padx=3)
  ttk.Button(c,text="Save current",command=self.save).pack(side="left",padx=(15,3))
  ttk.Button(c,text="Load reference",command=self.loadref).pack(side="left",padx=3)
  ttk.Button(c,text="Compare",command=self.compare).pack(side="left",padx=3)
  ttk.Separator(c,orient="vertical").pack(side="left",fill="y",padx=8)
  ttk.Button(c,text="RESTORE 00-0F",command=self.restore_00_0f).pack(side="left",padx=3)
  ttk.Button(c,text="RESTORE 00-3F",command=self.restore_00_3f).pack(side="left",padx=3)
  ttk.Button(c,text="ERASE EEPROM",command=self.erase_eeprom).pack(side="left",padx=3)
  ttk.Separator(c,orient="vertical").pack(side="left",fill="y",padx=10)
  ttk.Label(c,text="WRITE addr:").pack(side="left")
  self.wa=tk.StringVar(value="3F")
  ttk.Entry(c,textvariable=self.wa,width=4).pack(side="left",padx=3)
  ttk.Label(c,text="value:").pack(side="left")
  self.wv=tk.StringVar(value="FFFF")
  ttk.Entry(c,textvariable=self.wv,width=6).pack(side="left",padx=3)
  ttk.Button(c,text="WRITE + VERIFY",command=self.writeword).pack(side="left",padx=3)

  ca=ttk.LabelFrame(self.r,text="Calibration adjustment",padding=8);ca.pack(fill="x",padx=8,pady=(0,8))
  ttk.Label(ca,text="Select:").pack(side="left")
  self.calsel=tk.StringVar(value="CAL1")
  for k in ("CAL1","CAL2","CAL3","CAL4"):
   ttk.Radiobutton(ca,text=k,variable=self.calsel,value=k).pack(side="left",padx=(6,0))
  ttk.Label(ca,text="    Delta:").pack(side="left")
  self.caldelta=tk.StringVar(value="+100")
  ttk.Entry(ca,textvariable=self.caldelta,width=8).pack(side="left",padx=4)
  ttk.Label(ca,text="(e.g. +200 or -200)").pack(side="left",padx=(0,8))
  ttk.Button(ca,text="APPLY CAL Δ + BACKUPS",command=self.adjust_cal).pack(side="left",padx=4)
  self.calpreview=tk.StringVar(value="Uses current EEPROM value shown in GUI")
  ttk.Label(ca,textvariable=self.calpreview).pack(side="left",padx=12)

  p=ttk.Panedwindow(self.r,orient="horizontal");p.pack(fill="both",expand=True,padx=8)
  l=ttk.Frame(p);rr=ttk.Frame(p);p.add(l,weight=3);p.add(rr,weight=2)
  b=ttk.LabelFrame(l,text="EEPROM 93LC46B - 64 x 16 bit",padding=5);b.pack(fill="both",expand=True)
  cols=("a","h","d","r","chg","m");self.t=ttk.Treeview(b,columns=cols,show="headings")
  for k,n,w in zip(cols,("Addr","Current HEX","DEC","Reference","Change","Known use"),(55,85,80,85,80,170)):
   self.t.heading(k,text=n);self.t.column(k,width=w,anchor="center")
  self.t.column("m",anchor="w");self.t.tag_configure("chg",background="#ffe3a3")
  s=ttk.Scrollbar(b,command=self.t.yview);self.t.configure(yscrollcommand=s.set);self.t.pack(side="left",fill="both",expand=True);s.pack(side="right",fill="y")
  for a in range(64):self.t.insert("", "end",iid=f"{a:02X}",values=(f"{a:02X}","----","","----","",MEAN.get(a,"")))

  b=ttk.LabelFrame(rr,text="Decoded summary",padding=5);b.pack(fill="x")
  self.sum=tk.Text(b,height=13);self.sum.pack(fill="x")
  self.rv=tk.StringVar(value="Reference: none");ttk.Label(rr,textvariable=self.rv).pack(fill="x",pady=5)
  b=ttk.LabelFrame(rr,text="Serial terminal",padding=5);b.pack(fill="both",expand=True)
  self.term=tk.Text(b,wrap="none");self.term.pack(fill="both",expand=True)
  ttk.Label(self.r,text="CAL adjustment writes primary + both complement backups. RAW WRITE / RESTORE / ERASE remain available.",padding=8).pack(anchor="w")
  self.summary()

 def ports(self):
  x=[p.device for p in list_ports.comports()];self.pc["values"]=x
  if x and self.pv.get() not in x:self.pv.set(x[0])

 def conn(self):
  if self.ser and self.ser.is_open:self.disconn();return
  try:
   self.ser=serial.Serial(self.pv.get(),9600,bytesize=8,parity="N",stopbits=1,timeout=.05,write_timeout=1)
   self.ser.reset_input_buffer();self.ser.reset_output_buffer()
  except Exception as e:messagebox.showerror("Serial",str(e));return
  self.stop.clear();threading.Thread(target=self.reader,daemon=True).start()
  self.cb.config(text="Disconnect");self.st.set(f"Connected {self.pv.get()} @ 9600");self.log("[PC] Connected\n")
  # Automatic handshake: the first valid command changes ISL6 LCD
  # from PC WAITING to LINK ACTIVE.
  self.r.after(300,lambda:self.send("PING"))
  self.r.after(900,lambda:self.send("INFO"))

 def disconn(self):
  # A user-requested Disconnect also asks the charger to restart.
  # Send the command first, then close the COM port after it has left the PC.
  if self.ser and self.ser.is_open:
   try:
    self.log("\n[PC -> ISL6] RESET (Disconnect)\n")
    for ch in "RESET":
     self.ser.write(ch.encode("ascii"));self.ser.flush();time.sleep(.020)
    self.ser.write(b"\r");self.ser.flush()
    time.sleep(.20)
   except Exception as e:
    self.log(f"\n[PC] RESET send failed: {e}\n")

  self.stop.set()
  try:
   if self.ser:self.ser.close()
  except:pass
  self.ser=None;self.cb.config(text="Connect");self.st.set("Disconnected / ISL6 restarting")
  self.log("[PC] Disconnected - restart requested\n")

 def reader(self):
  while not self.stop.is_set():
   try:
    x=self.ser.read(256)
    if x:self.q.put(x.decode("ascii",errors="replace"))
   except Exception as e:self.q.put(f"\n[Serial error] {e}\n");return

 def poll(self):
  try:
   while 1:
    x=self.q.get_nowait();self.log(x);self.parse(x)
  except queue.Empty:pass
  self.r.after(40,self.poll)

 def send(self,x):
  if not self.ser or not self.ser.is_open:messagebox.showwarning("Serial","Not connected.");return
  try:
   self.log(f"\n[PC -> ISL6] {x}\n")
   for ch in x:self.ser.write(ch.encode());self.ser.flush();time.sleep(.020)
   self.ser.write(b"\r");self.ser.flush()
  except Exception as e:messagebox.showerror("Serial",str(e))

 def readone(self):
  try:a=int(self.av.get(),16)
  except:messagebox.showwarning("READ","Use hexadecimal address 00..3F.");return
  if not 0<=a<=63:messagebox.showwarning("READ","Address must be 00..3F.");return
  self.av.set(f"{a:02X}");self.send(f"READ {a:02X}")

 def parse(self,x):
  self.buf+=x
  while "\n" in self.buf:
   ln,self.buf=self.buf.split("\n",1);ln=ln.rstrip("\r")
   mm=re.match(r"^MODE:\s*([12])\s*$",ln,re.I)
   if mm:
    self.hwmode=mm.group(1);self._update_hwinfo();continue
   mm=re.match(r"^MODEL:\s*(.+?)\s*$",ln,re.I)
   if mm:
    self.model=mm.group(1);self._update_hwinfo();continue
   if "EEPROM 93LC46B DUMP 00-3F" in ln:self.indump=True;continue
   if self.indump and "EEPROM DUMP DONE" in ln:self.indump=False;self.table();self.summary();continue
   m=DUMP_RE.match(ln)
   if self.indump and m:
    a=int(m.group(1),16);self.cur[a]=int(m.group(2),16);continue
   m=READ_RE.search(ln)
   if m:self.cur[int(m.group(1),16)]=int(m.group(2),16);self.table();self.summary()

 def _update_hwinfo(self):
  if self.hwmode in ("1","2"):
   self.hwinfo.set(f"Hardware: MODE {self.hwmode} — {self.model}")
  else:
   self.hwinfo.set(f"Hardware: {self.model}")

 def calok(self,k):
  p,a,b=CAL[k]
  if None in (self.cur[p],self.cur[a],self.cur[b]):return None
  return self.cur[a]==(self.cur[p]^0xffff) and self.cur[b]==(self.cur[p]^0xffff)

 def summary(self):
  q=[];v=self.cur[10];q.append("Serial Number : ----" if v is None else f"Serial Number : {v} (0x{v:04X})");q.append("")
  for k,(p,_,__) in CAL.items():
   v=self.cur[p];o=self.calok(k);st="----" if o is None else ("OK" if o else "FAULT")
   q.append(f"{k}: ---- {st}" if v is None else f"{k}: {v} (0x{v:04X})  {st}")
  q+=["","Unknown EEPROM words:"]
  for a in (0,2,15):
   v=self.cur[a];q.append(f"  0x{a:02X}: ----" if v is None else f"  0x{a:02X}: 0x{v:04X} / {v}")
  if self.refname:
   n=sum(c is not None and r is not None and c!=r for c,r in zip(self.cur,self.ref));q+=["",f"Changed vs reference: {n} word(s)"]
  self.sum.delete("1.0","end");self.sum.insert("1.0","\n".join(q))

 def table(self):
  for a in range(64):
   c,r=self.cur[a],self.ref[a];ch=""
   if c is not None and r is not None:ch="=" if c==r else f"{c-r:+d}"
   self.t.item(f"{a:02X}",values=(f"{a:02X}","----" if c is None else f"{c:04X}","" if c is None else c,
    "----" if r is None else f"{r:04X}",ch,MEAN.get(a,"")),tags=("chg",) if c is not None and r is not None and c!=r else ())

 def parsefile(self,p):
  z=[None]*64
  for ln in Path(p).read_text(encoding="utf-8",errors="replace").splitlines():
   m=DUMP_RE.match(ln)
   if m:
    a=int(m.group(1),16)
    if a<64:z[a]=int(m.group(2),16)
  return z

 def loadref(self):
  p=filedialog.askopenfilename(filetypes=[("Text files","*.txt"),("All files","*.*")])
  if not p:return
  z=self.parsefile(p)
  if not any(v is not None for v in z):messagebox.showwarning("Reference","No AA VVVV EEPROM lines found.");return
  self.ref=z;self.refname=Path(p).name;self.rv.set("Reference: "+self.refname);self.table();self.summary()

 def compare(self):
  if not self.refname:messagebox.showwarning("Compare","Load reference first.");return
  d=[(a,r,c) for a,(c,r) in enumerate(zip(self.cur,self.ref)) if c is not None and r is not None and c!=r]
  self.table();self.summary()
  if not d:messagebox.showinfo("Compare","No differences found.");return
  messagebox.showinfo("EEPROM differences","\n".join(f"{a:02X}: {r:04X} -> {c:04X}  ({c-r:+d})" for a,r,c in d))

 def writeword(self):
  a=self.wa.get().strip().upper()
  v=self.wv.get().strip().upper()

  if not re.fullmatch(r"[0-9A-F]{2}",a):
   messagebox.showwarning("EEPROM WRITE","Address must be exactly 2 hexadecimal digits (00-3F).")
   return

  ai=int(a,16)
  if ai>0x3F:
   messagebox.showwarning("EEPROM WRITE","Address must be in range 00-3F.")
   return

  if not re.fullmatch(r"[0-9A-F]{4}",v):
   messagebox.showwarning("EEPROM WRITE","Value must be exactly 4 hexadecimal digits.")
   return

  current=self.eep.get(ai) if hasattr(self,"eep") else None

  warning=(
   f"Write EEPROM word 0x{a} = 0x{v}?\n\n"
   "The firmware will write the word and immediately verify it by readback."
  )

  # Active ISL6 data lives in 00..0F. Give a stronger warning there.
  if ai<=0x0F:
   warning+=(
    "\n\nWARNING: 0x00-0x0F is the active ISL6 data area."
    "\nCalibration, serial number or charger settings may be changed."
   )

  if current is not None:
   warning+=f"\n\nCurrent GUI value: 0x{current:04X}"

  if not messagebox.askyesno("Confirm EEPROM write",warning):
   return

  self.send(f"WRITE {a} {v}")
  # Read the same address again after the firmware write/readback sequence.
  self.r.after(800,lambda:self.send(f"READ {a}"))

 def adjust_cal(self):
  if not self.ser or not self.ser.is_open:
   messagebox.showwarning("CAL adjustment","Not connected.")
   return

  k=self.calsel.get()
  if k not in CAL:
   messagebox.showwarning("CAL adjustment","Select CAL1..CAL4.")
   return

  s=self.caldelta.get().strip().replace(" ","")
  if not re.fullmatch(r"[+-]?\d+",s):
   messagebox.showwarning("CAL adjustment","Delta must be a signed decimal number, e.g. +200 or -200.")
   return

  delta=int(s,10)
  p,c1,c2=CAL[k]
  old=self.cur[p]

  if old is None:
   messagebox.showwarning(
    "CAL adjustment",
    f"{k} current value is not known.\n\nRun DUMP EEPROM first."
   )
   return

  new=old+delta
  if not 0 <= new <= 0xFFFF:
   messagebox.showwarning(
    "CAL adjustment",
    f"Result is outside 16-bit range.\n\n{old} {delta:+d} = {new}"
   )
   return

  comp=new ^ 0xFFFF

  # Show whether the currently loaded triplet is internally consistent.
  ok=self.calok(k)
  state="OK" if ok is True else ("FAULT" if ok is False else "UNKNOWN")

  text=(
   f"{k} adjustment\n\n"
   f"Current primary : 0x{old:04X} / {old}\n"
   f"Delta           : {delta:+d}\n"
   f"New primary     : 0x{new:04X} / {new}\n"
   f"Complement      : 0x{comp:04X}\n\n"
   f"EEPROM writes:\n"
   f"  {p:02X} = {new:04X}   primary\n"
   f"  {c1:02X} = {comp:04X}   complement\n"
   f"  {c2:02X} = {comp:04X}   complement\n\n"
   f"Current redundancy: {state}\n\n"
   "Write this CAL triplet?"
  )

  if not messagebox.askyesno("Confirm CAL adjustment",text):
   return

  self.log(
   f"\n[PC] {k} ADJUST {delta:+d}: "
   f"{old} (0x{old:04X}) -> {new} (0x{new:04X})\n"
  )

  # Write primary plus both redundant inverted copies, then request a fresh dump.
  self._write_sequence([(p,new),(c1,comp),(c2,comp)],True)

  self.calpreview.set(
   f"{k}: {old} {delta:+d} -> {new}   (~ = {comp:04X})"
  )

 def _write_sequence(self,pairs,final_dump=True):
  if not self.ser or not self.ser.is_open:
   messagebox.showwarning("EEPROM","Not connected.")
   return

  # One EEPROM WRITE command at a time. 700 ms spacing leaves ample time
  # for slow character transmission, EEPROM internal write cycle and readback.
  delay=0
  for a,v in pairs:
   cmd=f"WRITE {a:02X} {v:04X}"
   self.r.after(delay,lambda c=cmd:self.send(c))
   delay+=700

  if final_dump:
   self.r.after(delay+500,lambda:self.send("DUMP"))

 def _restore_range(self,start,end):
  p=filedialog.askopenfilename(
   title=f"Restore EEPROM {start:02X}-{end:02X}",
   filetypes=[("Text files","*.txt"),("All files","*.*")]
  )
  if not p:return

  z=self.parsefile(p)
  missing=[a for a in range(start,end+1) if z[a] is None]
  if missing:
   messagebox.showerror(
    "Restore EEPROM",
    "Dump is incomplete.\n\nMissing addresses: "+
    " ".join(f"{a:02X}" for a in missing)
   )
   return

  preview="\n".join(f"{a:02X} {z[a]:04X}" for a in range(start,end+1))
  if len(preview)>1200:preview=preview[:1200]+"\n..."

  if not messagebox.askyesno(
   "Confirm EEPROM restore",
   f"Restore EEPROM 0x{start:02X}-0x{end:02X} from:\n{Path(p).name}\n\n"
   f"{preview}\n\n"
   "Each word will be written by ISL6 and verified by immediate readback.\n"
   "A final DUMP will be requested automatically."
  ):return

  self.log(f"\n[PC] RESTORE {start:02X}-{end:02X} from {p}\n")
  self._write_sequence([(a,z[a]) for a in range(start,end+1)],True)

 def restore_00_0f(self):
  self._restore_range(0x00,0x0F)

 def restore_00_3f(self):
  self._restore_range(0x00,0x3F)

 def erase_eeprom(self):
  if not self.ser or not self.ser.is_open:
   messagebox.showwarning("EEPROM","Not connected.")
   return

  if not messagebox.askyesno(
   "ERASE EEPROM",
   "This will overwrite ALL 64 EEPROM words 0x00-0x3F with 0xFFFF.\n\n"
   "The original ISL6 firmware will then see a blank EEPROM.\n\nContinue?"
  ):return

  if not messagebox.askyesno(
   "ERASE EEPROM - final confirmation",
   "FINAL CONFIRMATION\n\nWrite FFFF to every EEPROM address 00-3F?"
  ):return

  self.log("\n[PC] ERASE EEPROM 00-3F -> FFFF\n")
  self._write_sequence([(a,0xFFFF) for a in range(0x40)],True)

 def save(self):
  if not any(v is not None for v in self.cur):messagebox.showwarning("Save","No EEPROM data.");return
  n="ISL6_EEPROM_"+datetime.now().strftime("%Y-%m-%d_%H-%M-%S")+".txt"
  p=filedialog.asksaveasfilename(initialfile=n,defaultextension=".txt",filetypes=[("Text","*.txt")])
  if not p:return
  with open(p,"w",encoding="utf-8") as f:
   f.write("XTOOL for ISL6 - EEPROM dump\n\n")
   for a,v in enumerate(self.cur):f.write(f"{a:02X} {'----' if v is None else f'{v:04X}'}\n")
  self.log(f"\n[PC] Saved: {p}\n")

 def log(self,x):self.term.insert("end",x);self.term.see("end")
 def close(self):self.disconn();self.r.destroy()

if __name__=="__main__":
 root=tk.Tk();App(root);root.mainloop()
