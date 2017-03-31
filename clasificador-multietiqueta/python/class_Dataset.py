import xml.etree.ElementTree as et
import numpy as np
import os
import utils
import config

class Dataset:
	def __init__(self, path_data = "", batch=25):
		assert os.path.exists(path_data), 'No existe el archivo con los datos de entrada ' + path_data
		self.path_data = path_data
		self.texts_train = []
		self.labels_train = []
		self.batch = batch
		self.total_texts = len(self.texts_train)
		self.start = 0
		self.end = batch
		self.batch = []

	def read_data(self):
		print self.path_data
		xml = et.parse(self.path_data, et.XMLParser(encoding='ISO-8859-1')).getroot()
		extract_labels = False
		for reuters in xml.findall('REUTERS'):
			matrix = []
			for text in reuters.findall("TEXT"):
				body = utils.extract_body(text)
				if body != "" and body != None:
					extract_labels = True
					body = list(body.text)
				#if extract_labels == True:
					labels_temp = np.zeros(config.label_size)
					all_labels = 0
					for a_topic in reuters.findall("TOPICS"):
						for a_d in a_topic.findall("D"):
							try:
								label_index = utils.find_label_index(a_d.text)
								labels_temp[label_index] = 1.0
								all_labels += 1
							except ValueError:
								extract_labels = True
					for a_topic in reuters.findall("PLACES"):
						for a_d in a_topic.findall("D"):
							try:
								label_index = utils.find_label_index(a_d.text)
								labels_temp[label_index] = 1.0
								all_labels += 1
							except ValueError:
								extract_labels = True
					for a_topic in reuters.findall("PEOPLE"):
						for a_d in a_topic.findall("D"):
							try:
								label_index = utils.find_label_index(a_d.text)
								labels_temp[label_index] = 1.0
								all_labels += 1
							except ValueError:
								extract_labels = True
					for a_topic in reuters.findall("ORGS"):
						for a_d in a_topic.findall("D"):
							try:
								label_index = utils.find_label_index(a_d.text)
								labels_temp[label_index] = 1.0
								all_labels += 1
							except ValueError:
								extract_labels = True
					for a_topic in reuters.findall("EXCHANGES"):
						for a_d in a_topic.findall("D"):
							try:
								label_index = utils.find_label_index(a_d.text)
								labels_temp[label_index] = 1.0
								all_labels += 1
							except ValueError:
								extract_labels = True
					if all_labels != 0:
						if reuters.get("LEWISSPLIT") == "TRAIN":
							self.labels_train.append(labels_temp)
							matrix = utils.one_hot_encoder(body)
							self.texts_train.append(matrix)
						if reuters.get("LEWISSPLIT") == "TEST":
							config.labels_test.append(labels_temp)
							matrix = utils.one_hot_encoder(body)
							config.texts_test.append(matrix)
						extract_labels = False
					else:
						extract_labels = False
