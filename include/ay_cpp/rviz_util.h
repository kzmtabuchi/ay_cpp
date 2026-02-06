//-------------------------------------------------------------------------------------------
/*! \file    rviz_util.h
    \brief   RViz utilities.
    \author  Akihiko Yamaguchi, info@akihikoy.net
    \version 0.1
    \date    Dec.16, 2022
*/
//-------------------------------------------------------------------------------------------
#ifndef rviz_util_h
#define rviz_util_h
//-------------------------------------------------------------------------------------------
#include <ay_cpp/geom_util.h>
//-------------------------------------------------------------------------------------------
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/color_rgba.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <visualization_msgs/msg/marker_array.hpp>
//-------------------------------------------------------------------------------------------
#include <Eigen/Core>
#include <Eigen/Geometry>
//-------------------------------------------------------------------------------------------
#include <set>
#include <string>
//-------------------------------------------------------------------------------------------
namespace trick
{
//-------------------------------------------------------------------------------------------


// Utility for RViz.
class TSimpleVisualizer
{
private:
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr viz_pub_;

protected:
  rclcpp::Node::SharedPtr node_ptr_;
  int curr_id_;
  std::set<int> added_ids_;
  std::string viz_frame_;
  std::string viz_ns_;
  rclcpp::Duration viz_dt_;

  virtual void marker_operation(const visualization_msgs::msg::Marker &marker)
    {
      viz_pub_->publish(marker);
    }

public:
  typedef std_msgs::msg::ColorRGBA ColorRGBA;
  typedef geometry_msgs::msg::Vector3 Vector3;
  typedef geometry_msgs::msg::Pose Pose;
  typedef geometry_msgs::msg::Point Point;
  typedef geometry_msgs::msg::Quaternion Quaternion;
  typedef visualization_msgs::msg::Marker Marker;
  typedef visualization_msgs::msg::MarkerArray MarkerArray;

  TSimpleVisualizer() : viz_dt_(0, 0) {}

  virtual void Setup(rclcpp::Node::SharedPtr node,const rclcpp::Duration &viz_dt=rclcpp::Duration(0,0), const std::string &name_space="visualizer",
             const std::string &frame="", int queue_size=1, const std::string &topic="visualization_marker")
    {
      node_ptr_ = node;
      if(topic!="")
      {
        auto qos = rclcpp::QoS(rclcpp::KeepLast(queue_size)).reliable().transient_local();
        viz_pub_ = node->create_publisher<Marker>(topic, qos);
      }
      curr_id_= 0;
      added_ids_.clear();
      viz_frame_= frame==""?"base":frame;
      viz_ns_= name_space;
      viz_dt_= viz_dt;
    }

  ~TSimpleVisualizer()
    {
      if(viz_dt_!=rclcpp::Duration(0,0))
        DeleteAllMarkers();
      this->Reset();
      if (viz_pub_)
        viz_pub_.reset();
    }

  const std::string& Frame() const {return viz_frame_;}
  const std::string& Namespace() const {return viz_ns_;}
  const rclcpp::Duration& Dt() const {return viz_dt_;}
  void SetFrame(const std::string &v)  {viz_frame_= v;}
  void SetNamespace(const std::string &v)  {viz_ns_= v;}
  void SetDt(const rclcpp::Duration &v)  {viz_dt_= v;}

  virtual void Reset()
    {
      curr_id_= 0;
    }

  virtual void DeleteMarker(int mid)
    {
      Marker marker;
      marker.header.frame_id= viz_frame_;
      marker.ns= viz_ns_;
      marker.id= mid;
      marker.action= Marker::DELETE;
      marker_operation(marker);
      added_ids_.erase(mid);
    }

  virtual void DeleteAllMarkers()
    {
      Marker marker;
      marker.header.frame_id= viz_frame_;
      marker.ns= viz_ns_;
      marker.action= Marker::DELETEALL;
      marker_operation(marker);
      added_ids_.clear();
    }

  ColorRGBA Col(const float &r, const float &g, const float &b) const
    {
      return GenGRBGA<ColorRGBA>(r,g,b);
    }

  ColorRGBA ICol(int i) const
    {
      i= i%7;
      switch(i)
      {
        case 0:  return GenGRBGA<ColorRGBA>(1,0,0, 1);
        case 1:  return GenGRBGA<ColorRGBA>(0,1,0, 1);
        case 2:  return GenGRBGA<ColorRGBA>(0,0,1, 1);
        case 3:  return GenGRBGA<ColorRGBA>(1,1,0, 1);
        case 4:  return GenGRBGA<ColorRGBA>(1,0,1, 1);
        case 5:  return GenGRBGA<ColorRGBA>(0,1,1, 1);
        case 6:  return GenGRBGA<ColorRGBA>(1,1,1, 1);
      }
      return GenGRBGA<ColorRGBA>();
    }

  Marker GenMarker(const Pose &x, const Vector3 &scale, const ColorRGBA &rgb, const float &alpha) const
    {
      Marker marker;
      marker.header.frame_id= viz_frame_;
      marker.header.stamp= node_ptr_->now();
      marker.ns= viz_ns_;
      marker.action= Marker::ADD;
      marker.lifetime= viz_dt_;
      marker.scale= scale;
      marker.color= GenGRBGA<ColorRGBA>(rgb.r,rgb.g,rgb.b,alpha);
      marker.pose= x;
      return marker;
    }

  int SetID(Marker &marker, int mid)
    {
      if(mid<0)
      {
        marker.id= curr_id_;
        curr_id_++;
      }
      else
      {
        marker.id= mid;
        if(marker.id>=curr_id_)
          curr_id_= marker.id+1;
      }
      added_ids_.insert(marker.id);
      return marker.id+1;
    }

//   #Visualize a marker at x.  If mid is None, the id is automatically assigned
//   def AddMarker(self, x, scale=[0.02,0.02,0.004], rgb=[1,1,1], alpha=1.0, mid=None):
//     marker= self.GenMarker(x, scale, rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.CUBE  # or CUBE, SPHERE, ARROW, CYLINDER
//     marker_operation(marker)
//     return mid2

  // Visualize an arrow at x.  If mid is None, the id is automatically assigned
  int AddArrow(const Pose &x, const Vector3 &scale=GenGPoint<Vector3>(0.05,0.002,0.002), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::ARROW;
      marker_operation(marker);
      return mid2;
    }

//   #Visualize an arrow from p1 to p2.  If mid is None, the id is automatically assigned
//   def AddArrowP2P(self, p1, p2, diameter=0.002, rgb=[1,1,1], alpha=1.0, mid=None):
//     x= XFromP1P2(p1, p2, ax='x', r=0)
//     length= la.norm(Vec(p2)-Vec(p1))
//     scale= [length,diameter,diameter]
//     marker= self.GenMarker(x, scale, rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.ARROW  # or CUBE, SPHERE, ARROW, CYLINDER
//     marker_operation(marker)
//     return mid2

//   #Visualize a list of arrows.  If mid is None, the id is automatically assigned
//   def AddArrowList(self, x_list, axis='x', scale=[0.05,0.002], rgb=[1,1,1], alpha=1.0, mid=None):
//     iex,iey= {'x':(0,1),'y':(1,2),'z':(2,0)}[axis]
//     def point_on_arrow(x,l):
//       exyz= RotToExyz(QToRot(x[3:]))
//       ex,ey= exyz[iex],exyz[iey]
//       pt= x[:3]+l*ex
//       return [x[:3], pt, pt, x[:3]+0.7*l*ex+0.15*l*ey, pt, x[:3]+0.7*l*ex-0.15*l*ey]
//     x= [0,0,0, 0,0,0,1]
//     if not isinstance(rgb[0],(int,float)):  rgb= np.repeat(rgb,6,axis=0)
//     marker= self.GenMarker(x, [scale[1],0.0,0.0], rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.LINE_LIST
//     marker.points= [geometry_msgs.msg.Point(*p) for x in x_list for p in point_on_arrow(x,scale[0])]
//     marker_operation(marker)
//     return mid2

  // Visualize a cube at x.  If mid is None, the id is automatically assigned
  int AddCube(const Pose &x, const Vector3 &scale=GenGPoint<Vector3>(0.05,0.03,0.03), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::CUBE;
      marker_operation(marker);
      return mid2;
    }

//   #Visualize a list of cubes [[x,y,z]*N].  If mid is None, the id is automatically assigned
//   def AddCubeList(self, points, scale=[0.05,0.03,0.03], rgb=[1,1,1], alpha=1.0, mid=None):
//     x= [0,0,0, 0,0,0,1]
//     marker= self.GenMarker(x, scale, rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.CUBE_LIST
//     marker.points= [geometry_msgs.msg.Point(*p[:3]) for p in points]
//     marker_operation(marker)
//     return mid2

  // Visualize a sphere at x.  If mid is None, the id is automatically assigned
  int AddSphere(const Pose &x, const Vector3 &scale=GenGPoint<Vector3>(0.05,0.05,0.05), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::SPHERE;
      marker_operation(marker);
      return mid2;
    }
  // Visualize a sphere at p=[x,y,z].  If mid is None, the id is automatically assigned
  int AddSphere(const Point &p, const Vector3 &scale=GenGPoint<Vector3>(0.05,0.05,0.05), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Pose x;
      x.position= p;
      x.orientation= GenGQuaternion<Quaternion>(0.,0.,0.,1.);
      return AddSphere(x, scale, rgb, alpha, mid);
    }

//   #Visualize a list of spheres [[x,y,z]*N].  If mid is None, the id is automatically assigned
//   def AddSphereList(self, points, scale=[0.05,0.05,0.05], rgb=[1,1,1], alpha=1.0, mid=None):
//     x= [0,0,0, 0,0,0,1]
//     marker= self.GenMarker(x, scale, rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.SPHERE_LIST
//     marker.points= [geometry_msgs.msg.Point(*p[:3]) for p in points]
//     marker_operation(marker)
//     return mid2

  // Visualize a cylinder whose end points are p1 and p2.  If mid is None, the id is automatically assigned
  int AddCylinder(const Point &p1, const Point &p2, const float &diameter, const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      typedef Eigen::Matrix<float,3,1> EVec3;
      float ap1[3], ap2[3], pose[7];
      GPointToP(p1, ap1);
      GPointToP(p2, ap2);
      XFromP1P2(ap1, ap2, /*x_out=*/pose, /*ax=*/'z', /*r=*/0.5f);
      Pose x;
      XToGPose(pose, x);
      float length= (EVec3(ap2)-EVec3(ap1)).norm();

      Vector3 scale= GenGPoint<Vector3>(diameter,diameter,length);
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::CYLINDER;
      marker_operation(marker);
      return mid2;
    }

//   #Visualize a cylinder at x along its axis ('x','y','z').  If mid is None, the id is automatically assigned
//   def AddCylinderX(self, x, axis, diameter, l1, l2, rgb=[1,1,1], alpha=1.0, mid=None):
//     e= RotToExyz(QToRot(x[3:]))[{'x':0,'y':1,'z':2}[axis]]
//     p1= x[:3]+l1*e
//     p2= x[:3]+l2*e
//     return self.AddCylinder(p1,p2, diameter, rgb=rgb, alpha=alpha, mid=mid)

  // Visualize a points [[x,y,z]*N].  If mid is None, the id is automatically assigned
  int AddPoints(const std::vector<Point> &points, const Vector3 &scale=GenGPoint<Vector3>(0.03,0.03), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Pose x;
      x.position= GenGPoint<Point>(0.,0.,0.);
      x.orientation= GenGQuaternion<Quaternion>(0.,0.,0.,1.);
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::POINTS;
      marker.points= points;
      marker_operation(marker);
      return mid2;
    }

//   #Visualize a coordinate system at x with arrows.  If mid is None, the id is automatically assigned
//   def AddCoord(self, x, scale=[0.05,0.002], alpha=1.0, mid=None):
//     scale= [scale[0],scale[1],scale[1]]
//     p,R= XToPosRot(x)
//     Ry= np.array([R[:,1],R[:,2],R[:,0]]).T
//     Rz= np.array([R[:,2],R[:,0],R[:,1]]).T
//     mid= self.AddArrow(x, scale=scale, rgb=self.ICol(0), alpha=alpha, mid=mid)
//     mid= self.AddArrow(PosRotToX(p,Ry), scale=scale, rgb=self.ICol(1), alpha=alpha, mid=mid)
//     mid= self.AddArrow(PosRotToX(p,Rz), scale=scale, rgb=self.ICol(2), alpha=alpha, mid=mid)
//     return mid

//   #Visualize a coordinate system at x with cylinders.  If mid is None, the id is automatically assigned
//   def AddCoordC(self, x, scale=[0.05,0.002], alpha=1.0, mid=None):
//     scale= [scale[0],scale[1],scale[1]]
//     p,R= XToPosRot(x)
//     Ry= np.array([R[:,1],R[:,2],R[:,0]]).T
//     Rz= np.array([R[:,2],R[:,0],R[:,1]]).T
//     mid= self.AddCylinderX(x, 'x', scale[1], 0, scale[0], rgb=self.ICol(0), alpha=alpha, mid=mid)
//     mid= self.AddCylinderX(x, 'y', scale[1], 0, scale[0], rgb=self.ICol(1), alpha=alpha, mid=mid)
//     mid= self.AddCylinderX(x, 'z', scale[1], 0, scale[0], rgb=self.ICol(2), alpha=alpha, mid=mid)
//     return mid

  // Visualize a polygon [[x,y,z]*N].  If mid is None, the id is automatically assigned
  int AddPolygon(const std::vector<Point> &points, const Vector3 &scale=GenGPoint<Vector3>(0.02), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Pose x;
      x.position= GenGPoint<Point>(0.,0.,0.);
      x.orientation= GenGQuaternion<Quaternion>(0.,0.,0.,1.);
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::LINE_STRIP;
      marker.points= points;
      marker_operation(marker);
      return mid2;
    }

  // Visualize a list of lines [[x,y,z]*N] (2i-th and (2i+1)-th points are pair).  If mid is None, the id is automatically assigned
  int AddLineList(const std::vector<Point> &points, const Vector3 &scale=GenGPoint<Vector3>(0.02), const ColorRGBA &rgb=GenGRBGA<ColorRGBA>(), const float &alpha=1.0, int mid=-1)
    {
      Pose x;
      x.position= GenGPoint<Point>(0.,0.,0.);
      x.orientation= GenGQuaternion<Quaternion>(0.,0.,0.,1.);
      Marker marker= GenMarker(x, scale, rgb, alpha);
      int mid2= SetID(marker,mid);
      marker.type= Marker::LINE_LIST;
      marker.points= points;
      marker_operation(marker);
      return mid2;
    }

//   #Visualize a list of lines [[x,y,z]*N] (2i-th and (2i+1)-th points are pair).  If mid is None, the id is automatically assigned
//   def AddLineList(self, points, scale=[0.02], rgb=[1,1,1], alpha=1.0, mid=None):
//     x= [0,0,0, 0,0,0,1]
//     marker= self.GenMarker(x, list(scale)+[0.0,0.0], rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.LINE_LIST
//     marker.points= [geometry_msgs.msg.Point(*p[:3]) for p in points]
//     marker_operation(marker)
//     return mid2

//   #Visualize a text.  If mid is None, the id is automatically assigned
//   def AddText(self, p, text, scale=[0.02], rgb=[1,1,1], alpha=1.0, mid=None):
//     if len(p)==3:
//       x= list(p)+[0,0,0,1]
//     else:
//       x= p
//     marker= self.GenMarker(x, [0.0,0.0]+list(scale), rgb, alpha)
//     mid2= self.SetID(marker,mid)
//     marker.type= visualization_msgs.msg.Marker.TEXT_VIEW_FACING
//     marker.text= text
//     marker_operation(marker)
//     return mid2

//   #Visualize contacts which should be an moveit_msgs/ContactInformation[] [DEPRECATED:arm_navigation_msgs/ContactInformation[]]
//   def AddContacts(self, contacts, with_normal=False, scale=[0.01], rgb=[1,1,0], alpha=0.7, mid=None):
//     if len(contacts)==0:  return curr_id_
//     cscale= scale*3
//     viz_frame_= contacts[0].header.frame_id
//     for c in contacts:
//       p= [c.position.x, c.position.y, c.position.z]
//       mid= self.AddSphere(p+[0.,0.,0.,1.], scale=cscale, rgb=rgb, alpha=alpha, mid=mid)
//       if with_normal:
//         x= XFromP1P2(p, Vec(p)+Vec([c.normal.x, c.normal.y, c.normal.z]), ax='x', r=0.0)
//         ascale= [c.depth,0.2*scale[0],0.2*scale[0]]
//         mid= self.AddArrow(x, scale=ascale, rgb=rgb, alpha=alpha, mid=mid)
//     return mid

};
//-------------------------------------------------------------------------------------------

// Utility for RViz (MarkerArray version of TSimpleVisualizer).
class TSimpleVisualizerArray : public TSimpleVisualizer
{
private:
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr viz_pub_;

protected:
  MarkerArray marker_array_;

  /*override*/void marker_operation(const Marker &marker)
    {
      marker_array_.markers.push_back(marker);
    }

public:

  TSimpleVisualizerArray() {}

  /*override*/void Setup(rclcpp::Node::SharedPtr node, const rclcpp::Duration &viz_dt=rclcpp::Duration(0,0), const std::string &name_space="visualizer",
             const std::string &frame="", int queue_size=1, const std::string &topic="visualization_marker_array")
    {
      TSimpleVisualizer::Setup(node, viz_dt, name_space, frame, queue_size, /*topic=*/"");
      if(topic!="")
      {
        auto qos = rclcpp::QoS(rclcpp::KeepLast(queue_size)).reliable().transient_local();
        viz_pub_ = node->create_publisher<MarkerArray>(topic, qos);
      }
      Reset();
    }

  /*override*/void Reset()
    {
      curr_id_= 0;
      marker_array_= MarkerArray();
    }

  void Publish()
    {
      viz_pub_->publish(marker_array_);
      Reset();
    }

  /*override*/void DeleteAllMarkers()
    {
      Reset();
      Marker marker;
      marker.header.frame_id= viz_frame_;
      marker.ns= viz_ns_;
      marker.action= Marker::DELETEALL;
      marker_operation(marker);
      Publish();
      added_ids_.clear();
    }
};
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
}  // end of trick
//-------------------------------------------------------------------------------------------
#endif // rviz_util_h
//-------------------------------------------------------------------------------------------
