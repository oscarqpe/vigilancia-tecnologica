import numpy as np
import tensorflow as tf
import config

class Cnn:
	def __init__ (self) :
		# Parameters
		self.learning_rate = 0.001
		self.training_iters = 100
		self.batch_size = 128
		self.display_step = 10
		# Network Parameters
		self.n_input = config.vocabulary_size * config.max_characters # vocabulary * text character size (img shape: l * 924)
		self.n_classes = config.label_size # reuters total classes
		self.dropout = 0.5 # Dropout, probability to keep units
		# tf Graph input
		self.x = tf.placeholder(tf.float32, [1, self.n_input])
		self.y = tf.placeholder(tf.float32, [1, self.n_classes])
		self.keep_prob = tf.placeholder(tf.float32) #dropout (keep probability)
		# Store layers weight & bias
		self.weights = {
			# 5x5 conv, 1 input, 32 outputs
			'wc1': tf.Variable(tf.random_normal([7, config.vocabulary_size, 256])),
			# 5x5 conv, 32 inputs, 64 outputs
			'wc2': tf.Variable(tf.random_normal([7, 256, 256])),
			'wc3': tf.Variable(tf.random_normal([3, 256, 256])),
			'wc4': tf.Variable(tf.random_normal([3, 256, 256])),
			'wc5': tf.Variable(tf.random_normal([5, 256, 256])),
			'wc6': tf.Variable(tf.random_normal([3, 256, 256])),
			# fully connected, 7*7*64 inputs, 1024 outputs
			'wd1': tf.Variable(tf.random_normal([30*256, 1024])),
			'wd2': tf.Variable(tf.random_normal([1024, 1024])),
			# 1024 inputs, 10 outputs (class prediction)
			'out': tf.Variable(tf.random_normal([1024, config.label_size]))
		}
		self.biases = {
			'bc1': tf.Variable(tf.random_normal([256])),
			'bc2': tf.Variable(tf.random_normal([256])),
			'bc3': tf.Variable(tf.random_normal([256])),
			'bc4': tf.Variable(tf.random_normal([256])),
			'bc5': tf.Variable(tf.random_normal([256])),
			'bc6': tf.Variable(tf.random_normal([256])),
			'bd1': tf.Variable(tf.random_normal([1024])),
			'bd2': tf.Variable(tf.random_normal([1024])),
			'out': tf.Variable(tf.random_normal([config.label_size]))
		}
	# Create some wrappers for simplicity
	def convolution_1d(self, x, filters, bias, strides=1):
		# Conv1D wrapper, with bias and relu activation
		# x = tf.nn.conv2d(x, W, strides=[1, strides, strides, 1], padding='SAME')
		x = tf.nn.conv1d(x, filters, strides, padding='VALID')
		x = tf.nn.bias_add(x, bias)
		return tf.nn.relu(x)

	def max_pool_1d(self, x, lenght, output, k=2):
		# MaxPool2D wrapper
		x = tf.reshape(x, shape=[-1, 1, lenght, output])

		x = tf.nn.max_pool(x, ksize=[1, 1, k, 1], strides=[1, 1, k, 1],
		                      padding='VALID')
		x = tf.reshape(x, shape=[-1, lenght / k, output])
		return x

	def network(self, x, weights, biases, dropout):
		#print(x)
		da = tf.reshape(x, [-1])
		#print(da)
		input_data = tf.reshape(x, [1, config.max_characters, config.vocabulary_size])
		#print(input_data)
		#input_data = tf.Variable(da.astype(np.float32))
		#print(input_data)
		conv1 = self.convolution_1d(input_data, weights['wc1'], biases['bc1'], strides=1)
		#print(conv1)
		conv1 = self.max_pool_1d(conv1, 918, 256, 3)
		#print(conv1)

		conv2 = self.convolution_1d(conv1, weights['wc2'], biases['bc2'], strides=1)
		#print(conv2)
		conv2 = self.max_pool_1d(conv2, 306 - 7 + 1, 256, 3)
		#print(conv2)

		conv3 = self.convolution_1d(conv2, weights['wc3'], biases['bc3'], strides=1)
		#print(conv3)

		conv4 = self.convolution_1d(conv3, weights['wc4'], biases['bc4'], strides=1)
		#print(conv4)
		conv5 = self.convolution_1d(conv4, weights['wc5'], biases['bc5'], strides=1)
		#print(conv5)
		conv6 = self.convolution_1d(conv5, weights['wc6'], biases['bc6'], strides=1)
		#print(conv6)
		pool6 = self.max_pool_1d(conv6, 90, 256, 3)
		#print(pool6)
		# Fully connected layer
		# Reshape conv2 output to fit fully connected layer input
		#print(weights['wd1'])
		fc1 = tf.reshape(pool6, [-1, weights['wd1'].get_shape().as_list()[0]])
		fc1 = tf.add(tf.matmul(fc1, weights['wd1']), biases['bd1'])
		fc1 = tf.nn.sigmoid(fc1)
		#print(fc1)
		fc1 = tf.nn.dropout(fc1, dropout)
		#print(fc1)

		fc2 = tf.reshape(fc1, [-1, weights['wd2'].get_shape().as_list()[0]])
		fc2 = tf.add(tf.matmul(fc2, weights['wd2']), biases['bd2'])
		fc2 = tf.nn.sigmoid(fc2)
		#print(fc2)
		fc2 = tf.nn.dropout(fc2, dropout)
		#print(fc2)
		# Output, class prediction
		out = tf.add(tf.matmul(fc2, weights['out']), biases['out'])
		#out = tf.nn.sigmoid(out)
		return out