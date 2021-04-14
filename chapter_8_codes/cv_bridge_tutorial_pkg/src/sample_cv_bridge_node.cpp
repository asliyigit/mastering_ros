#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

static const std::string OPENCV_WINDOW = "Raw Image window";
static const std::string OPENCV_WINDOW_1 = "Edge Detection";

class Edge_Detector
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_; // ROS görüntü mesajlarını yayınlamak ve abone olmak için kullanılan bir ImageTransport örneği oluşturur.
  image_transport::Subscriber image_sub_; // ImageTransport sınıfının bir örneğini oluşturulur
  image_transport::Publisher image_pub_;// ImageTransport sınıfının bir örneğini oluşturulur
  
public:
  Edge_Detector()
    : it_(nh_)
  {
    // Subscribe to input video feed and publish output video feed
    //image_transport nesnesini kullanarak görüntülere abone olmak ve yayınlamak için Abone(subscribe) ve Yayıncı(advertise) nesnelerini oluşturulur
    image_sub_ = it_.subscribe("/usb_cam/image_raw", 1,  
      &Edge_Detector::imageCb, this);
    image_pub_ = it_.advertise("/edge_detector/raw_image", 1);
    
    //görsele nasıl abone oluyoruz ve yayınlıyor
    //image.cv::namedWindow (), bir görüntüyü görüntülemek için, GUI oluşturan bir OpenCV işlevidir. 
    //Bu işlevin içindeki argüman pencere adıdır. 
    cv::namedWindow(OPENCV_WINDOW); 

  }
  ~Edge_Detector()//Sınıfı yıkıcı içinde, adlandırılmış pencereyi ile yok edilir.
  {
    cv::destroyWindow(OPENCV_WINDOW);
  }


//Bu bir görüntü geri çağırma işlevidir ve temel olarak ROS görüntü mesajlarını CvBridge API’lerini kullanarak 
//OpenCV cv::Mat türüne dönüştürür. ROS’u OpenCV’ye ve tersini nasıl dönüştürebileceğimiz aşağıdadır:
  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {

    cv_bridge::CvImagePtr cv_ptr;//CvBridge ile başlamak için, bir CvImage örneği oluşturarak başlatılır
    namespace enc = sensor_msgs::image_encodings;

/*ROS görüntü mesajını iki şekilde ele alabiliriz: ya bir kopya yapabiliriz görüntünün veya görüntü verilerini paylaşabiliriz. 
 Resmi kopyaladığımızda, görüntüyü işler, ancak paylaşılan işaretçi kullanırsak, verileri değiştiremeyiz. 
 ROS görüntüsünün bir kopyasını oluşturmak için toCvCopy() kullanılır, 
 toCvShare() işlevi görüntünün işaretçisini elde etmek için kullanılır.*/ 
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);//ROS görüntüsünün bir kopyasını oluşturur
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    // Draw an example circle on the video stream
    /*Görüntünün satırlarının ve sütunlarının belirli bir aralıkta olup olmadığını kontrol eder ve 
    Eğer doğruysa, detect_edges (cv :: Mat) adında başka bir metodu çağırır ve parametre olarak verilen görüntüyü işleyin ve 
    kenar algılanan görüntüyü görüntüleyin.*/
    if (cv_ptr->image.rows > 400 && cv_ptr->image.cols > 600){

	detect_edges(cv_ptr->image);
    	image_pub_.publish(cv_ptr->toImageMsg());

	}
  }
  void detect_edges(cv::Mat img)
  {

   	cv::Mat src, src_gray;
	cv::Mat dst, detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 200;
	int highThreshold =300;
	int kernel_size = 5;

	img.copyTo(src);

	cv::cvtColor( img, src_gray, CV_BGR2GRAY );
        cv::blur( src_gray, detected_edges, cv::Size(5,5) );
	cv::Canny( detected_edges, detected_edges, lowThreshold, highThreshold, kernel_size );

  	dst = cv::Scalar::all(0);
  	img.copyTo( dst, detected_edges);
	dst.copyTo(img);

    	cv::imshow(OPENCV_WINDOW, src);
    	cv::imshow(OPENCV_WINDOW_1, dst);
    	cv::waitKey(3);

  }	
 
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "Edge_Detector");
  Edge_Detector ic;
  ros::spin();
  return 0;
}
