import xml.etree.ElementTree as et
import numpy as np
import tensorflow as tf
sess = tf.Session()

labels = []
with open("data/reuters/labels.txt", "r") as ins:
    for line in ins:
        labels.append(line.strip("\n"))
print(labels)
print("size labeles: ", len(labels))
label_size = len(labels)
e = et.parse('data/reuters/reut2-000.xml').getroot()
data_lenght = 924
outputSize = 256

# Print out each one-hot encoded string for 3 species.
vocabulario_alpha = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
     't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ',', '.',
     ';', ':', '!', '?', '\'', '"', '|', '/', '_', '@', '#', '$', '%', '&', '^', '~', '+', '-', '*',
     '=', '<', '>', '(', ')', '[', ']', '{', '}']

voc_size = len(vocabulario_alpha)

def one_hot (text):
	matrix = np.array([np.zeros(voc_size)])
	#print("text size: ", len(text))
	if len(text) > data_lenght:
		for i, val in enumerate(text):
			if i == data_lenght:
				break
			try:
				indice = vocabulario_alpha.index(val)
				array = np.zeros(voc_size)
				array[indice] = 1.0
				matrix = np.append(matrix, [array], axis = 0)
			except ValueError:
			    matrix = np.append(matrix, [np.zeros(voc_size)], axis = 0)
	else:
		j = 0
		for i, val in enumerate(text):
			j = i
			try:
				indice = vocabulario_alpha.index(val)
				array = np.zeros(voc_size)
				array[indice] = 1.0
				matrix = np.append(matrix, [array], axis = 0)
			except ValueError:
			    matrix = np.append(matrix, [np.zeros(voc_size)], axis = 0)
		#print("completar zeros: ", j)
		for i in range(j + 1, data_lenght):
			matrix = np.append(matrix, [np.zeros(voc_size)], axis = 0)
	matrix = np.delete(matrix, 0, 0)
	return matrix

def extractBody( text ):
   	for a_body in text.findall("BODY"):
   		if a_body.get("TYPE") is "BRIEF":
   			return None
   		return a_body
   	return ""
i = 0
data = []
data_labels = []

average = 0
extractlabels = False
for atype in e.findall('REUTERS'):
	# get body text
	#print("texto", i)
	matrix = []
	#if i == 2:
	#	break
	for a_text in atype.findall("TEXT"):
		body = extractBody(a_text)
		if body != "":
			average = average + len(body.text)
		if body != "" and body != None:
			extractlabels = True
			#print(len(body.text))
			body = list(body.text)
			#print("size body: ", len(body))
			matrix = one_hot(body)
			#print("size matrix: ", len(matrix))
			#print("size first element: ", len(matrix[0]))
			#print(matrix)
			#data.append(matrix)
			i = i + 1
	if extractlabels == True:
		#print("buscando etiquetas")
		label = np.zeros(label_size)
		total_etiquetas = 0
		for a_topic in atype.findall("TOPICS"):
			for a_d in a_topic.findall("D"):
				#print(a_d.text)
				try:
					indice = labels.index(a_d.text)
					#print("indice: ", indice)
					total_etiquetas = total_etiquetas + 1
					label[indice] = 1.0
				except ValueError:
				    l = 0
		for a_topic in atype.findall("PLACES"):
			for a_d in a_topic.findall("D"):
				#print(a_d.text)
				try:
					indice = labels.index(a_d.text)
					#print("indice: ", indice)
					total_etiquetas = total_etiquetas + 1
					label[indice] = 1.0
				except ValueError:
				    l = 0
		for a_topic in atype.findall("PEOPLE"):
			for a_d in a_topic.findall("D"):
				#print(a_d.text)
				try:
					indice = labels.index(a_d.text)
					total_etiquetas = total_etiquetas + 1
					label[indice] = 1.0
				except ValueError:
					l = 0
		for a_topic in atype.findall("ORGS"):
			for a_d in a_topic.findall("D"):
				#print(a_d.text)
				try:
					indice = labels.index(a_d.text)
					total_etiquetas = total_etiquetas + 1
					label[indice] = 1.0
				except ValueError:
					l = 0
		for a_topic in atype.findall("EXCHANGES"):
			for a_d in a_topic.findall("D"):
				#print(a_d.text)
				try:
					indice = labels.index(a_d.text)
					total_etiquetas = total_etiquetas + 1
					label[indice] = 1.0
				except ValueError:
					l = 0

		if total_etiquetas != 0:
			data_labels.append(label)
			data.append(matrix)
			extractlabels = False
		else:
			extractlabels = False
			#data = np.delete(data, len(data) - 1, 0)

print("Textos: ", len(data))
print("Etiquetas: ", len(data_labels))
print(data_labels[0])
"""for i in range(0,len(data)):
	print("x: ", len(data[i]))
	print("y: ", len(data[i][0]))
	#print(data[i])
print(data[0])
"""
print("end")