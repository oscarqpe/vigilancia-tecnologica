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
print config.vocabulary_size

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
config.training_iters = 40000
with tf.Session() as sess:
    sess.run(init)
    #print(sess.run(weights['wc1']))
    print("TRAINING")
    step = 0
    # Keep training until reach max iterations
    data = ds.Dataset("/home/oscarqpe/proyectos/qt/vigilancia-tecnologica/clasificador-multietiqueta/python/data/reuters/", config.batch_size)
    while step * config.batch_size < config.training_iters:
        data.next_batch()
        #data.read_data()
        data.generate_batch()
        #print data.texts_train.shape
        #print config.batch_size
        batch_x = data.texts_train
        batch_x = batch_x.reshape(config.batch_size, config.vocabulary_size * config.max_characters)
        #print batch_x.shape
        batch_y = data.labels_train
        batch_y = batch_y.reshape(config.batch_size, config.label_size)
        #print(len(batch_x), len(batch_x[0]))
        sess.run(optimizer, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: cnn.dropout})

        if step % 2 == 0:
            #print "Get Accuracy: "
            loss = sess.run([cost], feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1.})
            #print loss
            ou = sess.run(pred, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1})
            #print ou.shape
            #print batch_y.shape
            acc = utils.get_accuracy(ou, batch_y)
            #print(acc)
            print "Iter " + str(step * config.batch_size) + ", Minibatch Loss= " + \
                  str(loss[0]) + ", Training Accuracy= " + \
                  str(acc) + "/" + str(config.batch_size) + " correctos, " + str((acc * 100 / config.batch_size)) + " %"
        if data.end == data.total_texts:
            data.shuffler()
        step += 1
    print "TESTING"
    step = 0
    total_test = data.total_test
    print total_test
    true_positive = 0
    while step * config.batch_size < total_test:
        data.next_test()
        #data.read_data()
        data.generate_batch_test()
        #print data.texts_train.shape
        #print config.batch_size
        batch_x = data.texts_train
        batch_x = batch_x.reshape(config.batch_size, config.vocabulary_size * config.max_characters)
        #print batch_x.shape
        batch_y = data.labels_train
        batch_y = batch_y.reshape(config.batch_size, config.label_size)

        ou = sess.run(pred, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1})
        #print(ou)
        acc = utils.get_accuracy(ou, batch_y)
        loss = sess.run([cost], feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1.})
        #print loss
        ou = sess.run(pred, feed_dict={cnn.x: batch_x, cnn.y: batch_y, cnn.keep_prob: 1})
        #print ou.shape
        #print batch_y.shape
        acc = utils.get_accuracy(ou, batch_y)
        true_positive += acc
        #print(acc)
        print "Iter " + str(step * config.batch_size) + ", Minibatch Loss= " + \
              str(loss[0]) + ", Training Accuracy= " + \
              str(acc) + "/" + str(config.batch_size) + " correctos, " + str((acc * 100 / config.batch_size)) + " %"
        step += 1
    print "Total Accuracy: " + str (true_positive) + "/" + str(total_test) + ", " + str((true_positive * 100 / total_test)) + " %"
