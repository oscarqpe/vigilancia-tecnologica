class Dataset:
	def __init__(self, path_data = "", batch=25):
		assert os.path.exists(path_data), 'No existe el archivo con los datos de entrada ' + path_data
        self.path_data = path_data
        self.texts_train = []
        self.labels_train = []
        self.texts_test = []
        self.labels_test = []
        self.batch = batch
        self.total_texts = len(self.texts_train)
        self.start = 0
        self.end = batch
        self.batch = []
    
	def read_data(self):
		xml = et.parse(self.path_data, et.XMLParser(encoding='ISO-8859-1')).getroot()
		for reuters in xml.findall('REUTERS'):
			matrix = []
	        for text in reuters.findall("TEXT"):
	            body = extract_body(text)
	            if body != "" and body != None:
	                extract_labels = True
	                body = list(body.text)
	            if extract_labels == True:
	                labels_temp = np.zeros(label_size)
	                all_labels = 0
	                index_label = extract_label(reuters, "TOPICS")
	                if index_label != -1:
	                	labels_temp[index_label] = 1.0
	                	all_labels = all_labels + 1
                	index_label = extract_label(reuters, "PLACES")
	                if index_label != -1:
	                	labels_temp[index_label] = 1.0
	                	all_labels = all_labels + 1
	                index_label = extract_label(reuters, "PEOPLE")
	                if index_label != -1:
	                	labels_temp[index_label] = 1.0
	                	all_labels = all_labels + 1
	                index_label = extract_label(reuters, "ORGS")
	                if index_label != -1:
	                	labels_temp[index_label] = 1.0
	                	all_labels = all_labels + 1
	                index_label = extract_label(reuters, "EXCHANGES")
	                if index_label != -1:
	                	labels_temp[index_label] = 1.0
	                	all_labels = all_labels + 1
	                if all_labels != 0:
	                    if reuters.get("LEWISSPLIT") == "TRAIN":
	                        self.labels_train.append(labels_temp)
	                        matrix = one_hot_encoder(body)
	                        self.texts_train.append(matrix)
	                    if reuters.get("LEWISSPLIT") == "TEST":
	                        self.labels_test.append(labels_temp)
	                        matrix = one_hot_encoder(body)
	                        self.texts_test.append(matrix)
	                    extract_labels = False
	                else:
	                    extract_labels = False
