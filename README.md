# Robust SSAO

介绍：实现了GPU-Zen中的一篇文章（Robust Screen Space Ambient Occlusion in 1 ms in 1080p on PS4）

实现方式：OpenGL+VS

## 1 算法原理：

### 1.1 AAO算法：

&emsp;&emsp;该算法是基于 [Alcmy AO](https://research.nvidia.com/publication/alchemy-screen-space-ambient-obscurance-algorithm)算法，在AAO算法基础上加入了降低分辨率计算AO以及旋转采样的优化方法。

离散条件下，AAO算法的计算公式如下：
$$
\left.k_{A}=1-\frac{1}{s} \sum_{i=1}^{s} \frac{\max \left(0, \vec{v}_{i} \cdot \vec{n}+z_{c} \beta\right)}{\vec{v}_{i} \cdot \vec{v}_{i}+\varepsilon}\right), \vec{v}_{i}=p_{i}-p
$$
其中，$k_A$代表环境光照的输入（减去环境遮挡）。$s$是采样数量（蒙特卡洛）,采样空间是屏幕可见区域对应的三维区域。$v_i$是中心点到采样点的向量。$\varepsilon$是防止分母为0，一般取0.0001。偏移距离$\beta$是根据阴影映射的思路消除自阴影的影响。

&emsp;&emsp;由于该算法的采样空间可见区域（如下图$\Gamma$所示），因此可以直接在屏幕空间采样，重投影到三维空间点计算AO。在保证算法质量前提下，加快了计算速度。

<img src="https://gitee.com/huaiozhang/imagelib.git/RSSAO_AO%E8%AE%A1%E7%AE%97%E8%A1%A8%E7%A4%BA.jpg" style="zoom:67%;" />

&emsp;&emsp;上图展示了AAO计算场景的二维表示。公式（1）中定义的积分区域$\Gamma$如图所示，是采样半球与周围场景相交曲面对应立体角方向集合。区域$\Gamma$投影至屏幕空间后，是半径$r^{'}$ 的圆。在屏幕空间的此圆内采样即可计算AO。

### 1.2 RSSAO算法

&emsp;&emsp;在AAO基础上加入降低分辨率和优化采样方式即是RSSAO。

### 降低分辨率

&emsp;&emsp;在图像空间技术中，降低分辨率不仅可以加快计算速度，还可以保护图像边缘部分。RSSAO的降低分辨路计算如下图所示：

<img src="https://gitee.com/huaiozhang/imagelib.git/RSSAO_%E9%99%8D%E4%BD%8E%E5%88%86%E8%BE%A8%E7%8E%87%E6%8A%80%E6%9C%AF.jpg" style="zoom:67%;" />

&emsp;&emsp;其中，  AO模糊采用高斯模糊。上采样使用基于空间和AO的双边滤波。下采样后的深度缓冲是将原始非线性缓冲转化后线性缓冲（视野空间下的z值，未投影）。优势是线性缓冲不需要32位精度，降低内存，转化公式如下：
$$
z_{\text {eye}}=\frac{-p r o[3][2]}{2 * \operatorname{depth}+p r o[2][2]-1.0}
$$


### 随机旋转采样

&emsp;采样一个二维的螺旋模型。该螺旋模式结构保证了旋转样本坐标一定角度后，所有样本会与旋转前的样本坐标基本不重合，随机性大大增加。这种特征保证了AO效果只需要极少的采样点就可以取得不错的光照效果。在该采样方式中，[0，s-1]范围内的s个采样点，第i个采样点的计算坐标$u_i$:
$$
u_{i}=\left(r_{i} \cos \theta_{i}, r_{i} \sin \theta_{i}\right), r_{i}=\frac{\sqrt{i+0.5}}{\sqrt{s}}, \theta_{i}=2.4 i+\varphi
$$
其中，2.4为旋转黄金角度，$u_i$为旋转采样点角度。

为实现采样点坐标的半独立旋转，$u_i$ 值使用像素点的屏幕坐标计算，如下：
$$
\begin{array}{l}
\varphi=\operatorname{frac}\left(m_{z} \operatorname{frac}\left(w_{x y} \cdot m_{x y}\right)\right) \\
m=(0.06711056,0.0233486,52.9829189)
\end{array}
$$
其中，frac函数是截取小数后的小数，$\varphi$为屏幕坐标，m为给定常量参数。

## 2 算法流程

算法流程如下：

<img src="https://gitee.com/huaiozhang/imagelib.git/RSSAO_%E6%B8%B2%E6%9F%93%E6%B5%81%E7%A8%8B.JPG" style="zoom:50%;" />



## 3 算法效果

AAO算法，采样样本6，距离2如下：

<img src="https://gitee.com/huaiozhang/imagelib.git/AAO_%E9%87%87%E6%A0%B73%E8%B7%9D%E7%A6%BB2.jpg" style="zoom:33%;" />

RSSAO算法，采样样本3，距离2如下：

<img src="https://gitee.com/huaiozhang/imagelib.git/RSSAO_%E9%87%87%E6%A0%B73%E8%B7%9D%E7%A6%BB2.jpg" style="zoom:33%;" />

&emsp;&emsp;通过AAO和RRSSAO在采样数量为3，采样距离为2的对比下，RSSAO的AO图像在阴影影响范围、阴影细节、阴影的平滑以及真实度等方面明显优于AAO算法，已经获得明显的AO效果优势。尽管新算法使用双边滤波插值，AO图像仍然存在上图中如箭头所指的条纹噪声。但可以通过增加采样样本数量减少该噪声的影响，RSSAO算法，采样样本6，效果如下：

<img src="https://gitee.com/huaiozhang/imagelib.git/RSSAO_%E9%87%87%E6%A0%B76%E8%B7%9D%E7%A6%BB2.jpg" style="zoom: 33%;" />
