from sklearn import datasets
from sklearn.neural_network import MLPClassifier
import matplotlib.pyplot as plt
import numpy as np
import bonnerlib2
import pickle
from sklearn.preprocessing import StandardScaler
import sklearn as sl
import sklearn.utils as ut
'''Quetion 1  '''

#question 1.a)===================================================

def plotfig(X,t,title):
    #graph the data set with lable name as title
    color= np.array(['r','b'])
    plt.figure()
    plt.scatter(X[:, 0], X[:, 1], color=color[t],s=2)
    plt.suptitle(title)

'''
##############################
clf = MLPClassifier(hidden_layer_sizes=[3],
                             activation='tanh',
                             solver='sgd',
                             learning_rate_init=0.01,
                             tol=10.0**(-20),
                             max_iter=10000)
################################
'''
#creating train data and test data for the trainning process and test
#create 200 train data
Xtain,ttrain = datasets.make_moons(n_samples=200, noise=0.2)

Xtest,ttest = datasets.make_moons(n_samples=10000, noise=0.2)

plotfig(Xtain,ttrain,"Figure 1,Question 1(a) training set")
plotfig(Xtest,ttest,"Figute 2, Question 1(a)  testing set")
#plt.show()

#question 1.b)=========================================================





def fitMoons():
    figct = plt.figure()
    figct.suptitle("Figure 3, Question 1(b): contour plots for various training sessions.")
    mine = 1
    clfmin = 0
    for i in range(9):
        clf = MLPClassifier(hidden_layer_sizes=[3],
                        activation='tanh',
                        solver='sgd',
                        learning_rate_init=0.01,
                        tol=10.0 ** (-20),
                        max_iter=10000)
        clf.fit(Xtain,ttrain)
        acc = clf.score(Xtest,ttest)
        error = 1 - acc
        if error< mine:
            mine = error
            clfmin = clf
        print("The error is %f"%(error))
        ax = figct.add_subplot(3,3,i+1)
        ax.set_xlim(-4,4)
        ax.set_ylim(-4,4)
        colors=np.array(['r','b'])
        ax.scatter(Xtain[:,0],Xtain[:,1],color = colors[ttrain],s= 2)
        bonnerlib2.dfContour(clf,ax)
    print("The minimum error is %f"%(mine))
    fig1 = plt.figure()
    fig1.suptitle("Figure 4, Question 1(b): contour plot for best training session.")
    ax = fig1.add_subplot(1,1,1)
    ax.set_xlim(-4, 4)
    ax.set_ylim(-4, 4)
    colors = np.array(['r', 'b'])
    ax.scatter(Xtain[:, 0], Xtain[:, 1], color=colors[ttrain], s=2)
    bonnerlib2.dfContour(clfmin, ax)

#fitMoons()


#problem 3 a)
#from assgnment 2 import flatten
with open('mnist.pickle', 'rb') as f:
    data = pickle.load(f)

def flatten(data):
#return data marix x and class t
    X = []
    t = []
    for i in range(len(data)):
        for k in data[i]:
            X.append(k)
            t.append(i)
    return np.asarray(X),np.asarray(t)

X_train,t_train = flatten(data['training'])
X_test,t_test = flatten(data['testing'])

#Question 3b)###############################
X_train,t_train = ut.shuffle(X_train,t_train)





#Question 3c)##############################
scaler = StandardScaler()
scaler.fit(X_train)
X_train_norm = scaler.transform(X_train)
X_test_norm = scaler.transform(X_test)
def displaySample(N,D):
    #from my assignment 2
    newData = sl.utils.resample(np.asarray(D),replace = False)
    m = np.ceil(np.sqrt(N))
    figq2= plt.figure()
    for i in range(N):
        im = np.reshape(newData[i],(28,28))
        ax = figq2.add_subplot(m, m, i + 1)
        plt.axis('off')
        plt.imshow(im,cmap='Greys',interpolation='nearest')


#displaySample(16,X_test_norm)
plt.suptitle('Question 3(c): some normalized MNIST digits')


#Question 3d)############################
def find_best_lr_lv2(lr,momen,al):
        clf = MLPClassifier(hidden_layer_sizes=[100],
                            activation='tanh',
                            solver='sgd', batch_size=200,
                            max_iter=5,
                            warm_start=True,
                            tol=0.0,
                            learning_rate_init=lr,
                            momentum=momen,
                            alpha=al)
        min_test_error = 1
        for i in range(50):
                clf.fit(X_train_norm,t_train)
                test_error = 1 - clf.score(X_test_norm,t_test)
                print("Learning rate =%f, momen = %f alfa =%f :  The train error is %f and the Test error is %f"
                      %(lr,momen,al,1-clf.score(X_train_norm,t_train),1-clf.score(X_test_norm,t_test)))
        return test_error


find_best_lr_lv2(0.7,0.01,0.03)
find_best_lr_lv2(0.3,0.001,0.15)
#Learning rate =0.300000, momen = 0.001000 alfa =0.150000 :  The train error is 0.002717 and the Test error is 0.024700
#Learning rate =0.300000, momen = 0.00100 alfa =0.150000 :  The train error is 0.002317 and the Test error is 0.024100

'''
lr = [0.33,0.34,0.35,]
momen = [0.001,0.0015,0.002,0.0025]
alpha = [0.1,0.125,0.15,0.2]

min_error = 1
min_lr,min_momen,min_alpha = 0,0,0
for i in lr:
    for k in momen:
        for j in alpha:
            error = find_best_lr_lv2(i, k, j)
            if error<min_error:
                min_error = error
                min_lr = i
                min_momen = k
                min_alpha = j
            print(
            "The current best fit error is: %f , with lr :%f  momen:%f, alpha:%f" % (min_error, min_lr, min_momen, min_alpha))
print("The best fit error is: %f , with lr :%f  momen:%f, alpha:%f"%(min_error,min_lr,min_momen,min_alpha))
'''
# function for question 3 f) return the value of test error and train errors for graph

def find_best_lr_lv3(lr, momen, al):
    clf = MLPClassifier(hidden_layer_sizes=[100],
                        activation='tanh',
                        solver='sgd', batch_size=60000,
                        max_iter=1000,
                        warm_start=True,
                        tol=0.0,
                        learning_rate_init=lr,
                        momentum=momen,
                        alpha=al)
    train_error_list = []
    test_error_list = []
    min_test_error = 1
    for i in range(50):
        clf.fit(X_train_norm, t_train)
        test_error = 1 - clf.score(X_test_norm, t_test)
        erro = 1 - clf.score(X_train_norm, t_train)
        train_error_list.append(erro)
        test_error_list.append(test_error)


        print("Learning rate =%f, momen = %f alfa =%f :  The train error is %f and the Test error is %f"
              % (lr, momen, al, 1 - clf.score(X_train_norm, t_train), 1 - clf.score(X_test_norm, t_test)))
    return train_error_list,test_error_list


 find_best_lr_lv3(0.3,0.001,0.15)






#Question 4 a)
#define function predict
def predict(X,W1,W2,b1,b2):
    lay1 = np.dot(X,W1)+b1
    output1 = np.tanh(lay1)
    lay2 = np.dot(output1,W2)+b2
    output = np.exp(lay2)/np.sum(np.exp(lay2),axis=1,keepdims =True)
    return output1,output


#Quetion 4b)
clf = MLPClassifier(hidden_layer_sizes=[100],
                            activation='tanh',
                            solver='sgd', batch_size=200,
                            max_iter=5,
                            warm_start=True,
                            tol=0.0,
                            learning_rate_init=0.3,
                            momentum=0.001,
                            alpha=0.15)
clf.fit(X_train_norm,t_train)
W1= clf.coefs_[0]
W2= clf.coefs_[1]
b1 =clf.intercepts_[0]
b2 =clf.intercepts_[1]
#print(X_test_norm.shape)

#print(clf.intercepts_[0].shape)

#x2 is the print out and x1 is the hidden value

x1,x2 = predict(X_test_norm,W1,W2,b1,b2)
xx2 = clf.predict_proba(X_test_norm)
print("Question 4b) The difference between predict and predict_proba is")
print(str(np.mean((x2-xx2)**2)))


#Question 4c)
def gradient(H,Y,T):


   return 0
plt.show()








