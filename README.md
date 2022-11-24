# Demo

## Introduction

We implemented an automatic program for the fruit-cutting game(Fruit Ninja). We realized the identification of fruits and bombs based on the Scale Invariant Feature Transform(SIFT) algorithm, proposed a novel grid-search framework to generate decision paths, and finally carried out a series of optimizations to accelerate this program and reduce the potential effect of latency. Our work achieved better performance compared with normal human beings. 



## Basic Method

#### Preprocess-Grabcut

Grabcut is a classic and practical algorithm that can be used for foreground extraction, and we mainly use this algorithm to collect the original fruit and bomb information and then remove the potential noise in the background. We first took screenshots during the running process, and they were uniformly cut to a size of 100*100. Then, Grabcut method was used to extract the foreground and remove the background. Furthermore, To reduce the time cost, we decided to stitch all the fruit material pictures into one picture to match the screen shoots, and the bomb picture was matched separately. So, we must distinguish feature points corresponding to fruit or the bomb.

<img src="https://picbedofhsu.oss-cn-beijing.aliyuncs.com/img/fruits_without_bg.png" alt="fruits_without_bg" style="zoom: 50%;" />



### Interface

The interface provided by Windows is required for operation. First, it is necessary to get the precise screenshot of specified positions, zoom the source picture to perform mouse operations, and locate the pixels in the image.
We expect to further accelerate the overall operating efficiency to reduce system latency so that mouse operations can be performed based on the real-time location. This requirement is because the higher latency will cause the position of the fruit to change, resulting in inaccurate and invalid mouse operations. We also implement a series of optimization on the system level.

### Feature Matching

We will use the fruit material map and the bomb material map to match the sampling map based on SIFT feature extraction and ratio feature matching algorithm, save the successfully matched fruit feature points and bomb feature points in it, and pass them to the next module to determine the path to cut the fruits while avoiding bombs.

<img src="https://picbedofhsu.oss-cn-beijing.aliyuncs.com/img/pic2.jpg" alt="pic2" style="zoom: 80%;" />

Here we do not use the Grabcut algorithm to remove the background of the sampling image, mainly based on the following two considerations:

- Firstly, if the Grabcut algorithm is executed once for each sampling, a significant time delay will be generated;
- Secondly, the sampling image is generally large, and the distribution of fruits in it is usually scattered. The background removal may still be incomplete even if the Grabcut algorithm is used

<img src="https://picbedofhsu.oss-cn-beijing.aliyuncs.com/img/ea5ea1d13d3b7393ff5c1a85a44a518.jpg" alt="ea5ea1d13d3b7393ff5c1a85a44a518" style="zoom: 33%;" />



### Path Decision

Since the movement of the mouse will introduce additional noise in this game, and too much action will lead to high latency, so we propose a grid strategy to reduce action times and carry out cutting in a row, and this will also gain combos to earn bonus points under the mechanics of the game.

1. So we divide the whole screenshot into multiple grids, and for each grid, after the feature detection and matching, according to the number of points, we will divide all grids into three categories:
   1. The area containing the bomb feature point notated as -1
   2. The area containing enough fruit feature points notated as 1
   3. The area containing few feature points or no feature points notated as  0

So the strategy is: 
We need to avoid the first grid(-1), need to go through the second grid(1), and for the third(0), we can choose to pass or not pass, as shown below.

<img src="https://picbedofhsu.oss-cn-beijing.aliyuncs.com/img/strate.PNG" alt="strate" style="zoom: 67%;" />

This approach can ensure that as many fruits as possible can be segmented with fewer operations and avoid misoperations on additional factors such as bombs.



## Other Detail

According to the game mechanism we observed (a round of fruit will be thrown every 4s), we adopt the method of continuous sampling after waiting for 2s after each cut. At the same time, this method helps to eliminate the influence of mouse operation and fruit slices in the process of target point matching.

The experiment found that when the sampling image only contains the background, the feature points extracted by the feature extraction are very few, but when the fruit appears, the feature points will increase largely. So to further eliminate the influence of the background plate, a pre-screening method based on the number of matching points in the whole image can be implemented for the position where the number of feature points in the global image/local grid exceeds a certain threshold. Finally, we also noticed that setting sleep time after each operation also makes it unnecessary for us to consider the distinction between the states before and after mouse operation and the impact of the divided fruit on this task, which improves our framework's performance.

## 

