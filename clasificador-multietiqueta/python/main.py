import xml.etree.ElementTree as et
import numpy as np
import tensorflow as tf

import config
import utils
utils.read_labels()
import class_Dataset as ds
import cnn as cn


# print(labels)
print("Total labels: ", len(config.labels))

cnn = cn.Cnn()
# Construct model
pred = cnn.network(cnn.x, cnn.weights, cnn.biases, cnn.dropout)

# Define loss and optimizer
cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=pred, labels=cnn.y))

#cost = tf.reduce_mean(multilabel(logits=pred, labels=y))
optimizer = tf.train.AdamOptimizer(learning_rate=cnn.learning_rate).minimize(cost)

# Evaluate model
correct_pred = tf.equal(tf.argmax(pred, 1), tf.argmax(cnn.y, 1))
#accuracy = tf.reduce_mean(tf.cast(correct_pred, tf.float32))
#accuracy = get_accuracy(logits=pred, labels=y)
data = None
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
        for i in range(0, 2):
            ruta = utils.get_path_file(i)
            data = ds.Dataset(ruta, 25)
            data.read_data()
            print "Total Texts: " + str(len(data.texts_train))
            print "Total Texts Labels: " + str(len(data.labels_train))
            step = 0
            while step < len(data.texts_train):
                batch_x = data.texts_train[step]
                batch_y = data.labels_train[step]
                # Run optimization op (backprop)
                batch_x = np.asarray(batch_x).reshape(1, -1)
                batch_y = np.asarray(batch_y).reshape(1, -1)
                #print(len(batch_x), len(batch_x[0]))
                sess.run(optimizer, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: cnn.dropout})
                if step % 100 == 0:
                    loss = sess.run([cost], feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1.})
                    #print(loss[0])
                    ou = sess.run(pred, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1})
                    #print(ou)
                    acc = utils.get_accuracy(ou, batch_y)
                    #print(acc)
                    print "Iter " + str(step) + ", Minibatch Loss= " + \
                          str(loss[0]) + ", Training Accuracy= " + \
                          str(acc)
                step += 1
            data.texts_train = []
            data.labels_train = []
            step = 0
            presition = 0
            total_test = len(config.texts_test)
            print total_test
            while step < total_test:
                batch_x = config.texts_test[step]
                batch_y = config.labels_test[step]
                # Run optimization op (backprop)
                batch_x = np.asarray(batch_x).reshape(1, -1)
                batch_y = np.asarray(batch_y).reshape(1, -1)
                ou = sess.run(pred, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1})
                #print(ou)
                acc = get_accuracy(ou, batch_y)
                if acc == 1:
                    presition += 1
                step += 1
            if total_test > 0:
                print "Presition: " +  str(presition) + "/" + str(total_test) + " " + str(100 * presition / total_test) + " %"
        epoch += 1