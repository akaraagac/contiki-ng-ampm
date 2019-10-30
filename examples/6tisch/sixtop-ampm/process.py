import os
import time
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import random
import chart_studio.plotly as py
import plotly.graph_objects  as go
import numpy as np
from matplotlib.pyplot import imshow
import networkx as nx

process_list={}
ampm_data_list={"counters":[],"asns":[],"delays":[],"loss":[]}



def follow(name):
    current = open(name, "r")
    curino = os.fstat(current.fileno()).st_ino
    while True:
        while True:
            line = current.readline()
            if not line:
                break
            yield line

        try:
            if os.stat(name).st_ino != curino:
                new = open(name, "r")
                current.close()
                current = new
                curino = os.fstat(current.fileno()).st_ino
                continue
        except IOError:
            pass
        time.sleep(0.1)

def plot_ampm_telemetry():
	global ampm_data_list
	Delay1=[]
	Delay2=[]
	Delay3=[]
	Delay4=[]
	Loss1=[]
	Loss2=[]
	Loss3=[]
	Loss4=[]
	for i in range(len(ampm_data_list["delays"])):
		if(not ampm_data_list["delays"][i][0]==None):
			Delay1.append(ampm_data_list["delays"][i][0])
		else:
			Delay1.append(0)
		if(not ampm_data_list["delays"][i][1]==None):
			Delay2.append(ampm_data_list["delays"][i][1])
		else:
			Delay2.append(0)
		if(not ampm_data_list["delays"][i][2]==None):
			Delay3.append(ampm_data_list["delays"][i][2])		
		else:
			Delay3.append(0)
		if(not ampm_data_list["delays"][i][3]==None):
			Delay4.append(ampm_data_list["delays"][i][3])
		else:
			Delay4.append(0)

	for i in range(len(ampm_data_list["loss"])):
		if(not ampm_data_list["loss"][i][0]==None):
			Loss1.append(ampm_data_list["loss"][i][0])
		else:
			Loss1.append(0)
		if(not ampm_data_list["loss"][i][1]==None):
			Loss2.append(ampm_data_list["loss"][i][1])
		else:
			Loss2.append(0)
		if(not ampm_data_list["loss"][i][2]==None):
			Loss3.append(ampm_data_list["loss"][i][2])		
		else:
			Loss3.append(0)
		if(not ampm_data_list["loss"][i][3]==None):
			Loss4.append(ampm_data_list["loss"][i][3])
		else:
			Loss4.append(0)

	plt.figure(1)
	bar_fig.clear()
	ind = np.arange(len(Delay1)) 
	width=0.9;
	p1 = plt.bar(ind, Delay1,width)
	p2 = plt.bar(ind, Delay2,width,bottom=Delay1)
	p3 = plt.bar(ind, Delay3,width,bottom=[Delay1[i]+Delay2[i] for i in range(len(Delay1))])
	p4 = plt.bar(ind, Delay4,width,bottom=[Delay1[i]+Delay2[i]+Delay3[i] for i in range(len(Delay1))])
	plt.ylabel('Latency (slots)')
	plt.title('Hop-by-Hop Latency Values')
	if(len(p4)>0):
		plt.legend((p1[0], p2[0], p3[0], p4[0]), ('1st Hop', '2nd Hop', '3nd Hop', '4nd Hop'))
	bar_fig.canvas.flush_events()

	plt.figure(2)
	bar_fig2.clear()
	ind = np.arange(len(Loss1)) 
	p1 = plt.bar(ind, Loss1,width)
	p2 = plt.bar(ind, Loss2,width,bottom=Loss1)
	p3 = plt.bar(ind, Loss3,width,bottom=[Loss1[i]+Loss2[i] for i in range(len(Loss1))])
	p4 = plt.bar(ind, Loss4,width,bottom=[Loss1[i]+Loss2[i]+Loss3[i] for i in range(len(Loss1))])
	plt.ylabel('Losses (%)')
	plt.title('Packet Loss Values')
	plt.ylim((0,100))
	if(len(p4)>0):
		plt.legend((p1[0], p2[0], p3[0], p4[0]), ('1st Hop', '2nd Hop', '3nd Hop', '4nd Hop'))
	bar_fig2.canvas.flush_events()


asn_enrty=[None,None,None,None,None,None]
counter_enrty=[None,None,None,None,None,None]
lastcolor=0
firstreport=1
plotcount=0



def process_int_data():
	global process_list,ampm_data_list, asn_enrty,counter_enrty,lastcolor,firstreport,plotcount
	
	if not lastcolor==-1 and not (lastcolor == process_list["color"]) and not counter_enrty[2]==None:
		if(firstreport==1):
			firstreport=0
		else:	
			ampm_data_list["counters"].append(counter_enrty)
			ampm_data_list["asns"].append(asn_enrty)
			delays=[(asn_enrty[1]-asn_enrty[2]) if not (asn_enrty[1]==None or asn_enrty[2]==None) else None,(asn_enrty[0]-asn_enrty[1]) if not (asn_enrty[1]==None or asn_enrty[0]==None) else None,(asn_enrty[4]-asn_enrty[0]) if not (asn_enrty[4]==None or asn_enrty[0]==None) else None,(asn_enrty[5]-asn_enrty[4]) if not (asn_enrty[5]==None or asn_enrty[4]==None) else None]
			ampm_data_list["delays"].append(delays)
			for i in range(len(ampm_data_list["delays"][-1])):
				if(ampm_data_list["delays"][-1][i]<0):
					for j in range(i,len(ampm_data_list["delays"][-1])):
						ampm_data_list["delays"][-1][j]=None
					break
			ampm_data_list["loss"].append([100*float(counter_enrty[2]-counter_enrty[1])/counter_enrty[2] if not (counter_enrty[1]==None or counter_enrty[2]==None) else None,100*float(counter_enrty[1]-counter_enrty[0])/counter_enrty[2] if not (counter_enrty[1]==None or counter_enrty[0]==None) else None,100*float(counter_enrty[0]-counter_enrty[4])/counter_enrty[2] if not (counter_enrty[0]==None or counter_enrty[4]==None) else None, 100*float(counter_enrty[4]-counter_enrty[5])/counter_enrty[2] if not (counter_enrty[4]==None or counter_enrty[5]==None) else None])
			asn_enrty=[None,None,None,None,None,None]
			counter_enrty=[None,None,None,None,None,None]

			print ampm_data_list["delays"][-1], ampm_data_list["loss"][-1]
			#print "n3-- ",
			#print "delay: ", ampm_data_list["delays"][-1][0], " loss: ",ampm_data_list["loss"][-1][0],
			#print " --n2-- ",
			#print "delay: ", ampm_data_list["delays"][-1][1], " loss: ",ampm_data_list["loss"][-1][1],
			#print " --n1-- ",
			#print "delay: ", ampm_data_list["delays"][-1][2], " loss: ",ampm_data_list["loss"][-1][2],
			#print " --n5-- ",
			#print "delay: ", ampm_data_list["delays"][-1][3], " loss: ",ampm_data_list["loss"][-1][3],
			#print " --n6"
			plotcount = plotcount + 1
			if(plotcount>10):
				#plot_ampm_telemetry()
				plotcount=0

	asn_enrty[process_list["node"]-1]= process_list["asn"]
	counter_enrty[process_list["node"]-1]=process_list["counter"]
	
	lastcolor = process_list["color"]
	#print(process_list)


	

	
	
	
if __name__ == '__main__':
	fname1 = "log_ampm (0.5-2.0).txt"
	fname2 ="log_ampm (0.5-1).txt"
	fname3 ="log_ampm (0.2-2.0).txt"
	fname4 ="log_ampm (0.2-1.0).txt"
	fname5 ="log_ampm (0.1-1.0).txt"
	
	bar_fig=plt.figure(1)
	plt.ion() 
	plt.show()
	bar_fig.canvas.manager.window.wm_geometry("575x460+%d+%d" % (0, 520))

	bar_fig2=plt.figure(2)
	plt.ion() 
	plt.show()
	bar_fig2.canvas.manager.window.wm_geometry("575x460+%d+%d" % (0, 520))
	
	for line in follow(fname5):
			columns=line.split()
			process_list["timestamp"]=columns[0]
			process_list["node"]= int(columns[6])
			process_list["color"]= int(columns[10])
        		process_list["counter"]= int(columns[12])
        		process_list["asn"]= int(columns[14])
			process_int_data()

    
