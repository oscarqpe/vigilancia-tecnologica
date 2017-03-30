import xml.etree.ElementTree as et
import numpy as np
import tensorflow as tf

import config
import utils
import class_Dataset as ds

utils.read_labels()
# print(labels)
print("size labeles: ", len(config.labels))

init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    #print(sess.run(weights['wc1']))
    print("init")
    step = 0
    # Keep training until reach max iterations
    epoch = 0
    while epoch < 1:
        print "init epoch: " + str (epoch)
        for i in range(0, 1):
            ruta = utils.get_path_file(i)
            data = ds.Dataset(ruta, 25)
            data.read_data()
            print "Total Texts: " + str(len(data.texts_train))
            print "Total Texts Labels: " + str(len(data.labels_train))
        epoch += 1
