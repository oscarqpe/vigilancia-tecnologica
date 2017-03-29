import xml.etree.ElementTree as et
import numpy as np
import tensorflow as tf
import config
import utils
import class_Dataset as ds

utils.read_labels()
# print(labels)
print("size labeles: ", len(config.labels))


