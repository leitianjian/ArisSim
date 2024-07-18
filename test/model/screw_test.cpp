#include <iostream>

#include <gtest/gtest.h>

#include <aris.hpp>
using namespace aris::dynamic;

const static double identity[16]{1, 0, 0, 0, 0, 1, 0, 0,
                                 0, 0, 1, 0, 0, 0, 0, 1};
GTEST_TEST(Screw, axe_to_pm) {
  double pm[16], axis[3]{0, 1, 0}, position[3]{0, 0, 0};
  s_sov_axes2pm(position, axis, axis, pm, "zx");
  dsp(4, 4, pm);
  s_sov_pnts2pm(position, 1, axis, 1, axis, 1, pm, "zx");
  dsp(4, 4, pm);
}

// ����Model֮�����Ϣ����
GTEST_TEST(Motion, axis_of_motion) {
  // ��ʼ״̬�Ǵ�ֱ״̬
//   double a = 0.4;
//   double b = 0.4;
//   double c = 0.4;
//   const double PI = 3.1415926535897932384626433;

//   // ����ؽڵ�λ�ã��Լ����ߣ���3��ת���������߶���Z��
//   const double joint1_position[3]{0, 0, 0};
//   const double joint1_axis[3]{0, 0, 1};
//   const double joint2_position[3]{0, a, 0};
//   const double joint2_axis[3]{0, 0, 1};
//   const double joint3_position[3]{0, a + b, 0};
//   const double joint3_axis[3]{0, 0, 1};

//   // ����3���˼���λ����321ŷ���ǣ��Լ�10ά�Ĺ�������
//   // inertia_vectorΪ����������Ķ���Ϊ��[m, m*x, m*y, m*z, Ixx, Iyy, Izz,
//   // Ixy, Ixz, Iyz]������x,y,zΪ����λ��
//   const double link1_pos_euler[6]{0, a / 2, 0, PI / 2, 0, 0};
//   const double link1_intertia_vector[10]{
//       2, 0, 0, 0, 8.333333333338782e-04, 0.0271, 0.0271, 0, 0, 0};
//   const double link2_pos_euler[6]{0, a + b / 2, 0, PI / 2, 0, 0};
//   const double link2_intertia_vecter[10]{
//       2, 0, 0, 0, 8.333333333338782e-04, 0.0271, 0.0271, 0, 0, 0};
//   const double link3_pos_euler[6]{0, a + b + c / 2, 0, PI / 2, 0, 0};
//   const double link3_intertia_vecter[10]{
//       2, 0, 0, 0, 8.333333333338782e-04, 0.0271, 0.0271, 0, 0, 0};
//   const double body_intertia_vecter[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//   // ����ĩ��λ����321ŷ����
//   const double body_position_and_euler321[6]{0, a + b + c, 0, PI / 2, 0, 0};

//   // ����ģ��
//   aris::dynamic::Model model;

//   // ��������,������y��
//   const double gravity[6]{0.0, -9.81, 0.0, 0.0, 0.0, 0.0};
//   model.environment().setGravity(gravity);

//   // ��Ӹ˼�������pe����˼Ϊposition and euler
//   // angle�������Ĳ���ָ����λ���Լ���������
//   auto& link1 =
//       model.addPartByPe(link1_pos_euler, "321", link1_intertia_vector);
//   auto& link2 =
//       model.addPartByPe(link2_pos_euler, "321", link2_intertia_vecter);
//   auto& link3 =
//       model.addPartByPe(link3_pos_euler, "321", link3_intertia_vecter);

//   // ��ӹؽڣ����ת���ؽڣ�ǰ��������Ϊ�ؽ����ӵĸ˼������������������˹ؽڵ�λ��������
//   auto& joint1 = model.addRevoluteJoint(link1, model.ground(), joint1_position,
//                                         joint1_axis);
//   auto& joint2 =
//       model.addRevoluteJoint(link2, link1, joint2_position, joint2_axis);
//   auto& joint3 =
//       model.addRevoluteJoint(link3, link2, joint3_position, joint3_axis);

//   // ������� Joint1 Ϊ�����ؽڣ����ü�motion
//   // auto& motion1 = model.addMotion(joint1);
//   auto& motion2 = model.addMotion(joint2);
//   auto& motion3 = model.addMotion(joint3);
//   //
//   motion2.setAxis(3);
//   motion3.setAxis(4);

//   auto& force2 = model.forcePool().add<aris::dynamic::SingleComponentForce>(
//       "f2", motion2.makI(), motion2.makJ(), 5);
//   auto& force3 = model.forcePool().add<aris::dynamic::SingleComponentForce>(
//       "f3", motion3.makI(), motion3.makJ(), 5);

//   // ���ĩ�ˣ���һ����������ĩ��λ��link4�ϣ��ڶ�����������ĩ�˵�λ��������ڵ���ģ�����������������ĩ�˵���ʼλ��
//   auto& end_effector = model.addGeneralMotionByPe(
//       link3, model.ground(), body_position_and_euler321, "321");

//   //-------------------------------------------- ��������
//   //--------------------------------------------//
//   /// [Solver]
//   // ����������������Ϊ����������ڴ档ע�⣬�����һ�������ڴ���벻Ҫ����ӻ�ɾ���˼����ؽڡ�������ĩ�˵�����Ԫ��
//   auto& inverse_kinematic_solver =
//       model.solverPool().add<aris::dynamic::InverseKinematicSolver>();
//   auto& inverse_dynamic_solver =
//       model.solverPool().add<aris::dynamic::InverseDynamicSolver>();
//   auto& forward_kinematic_solver =
//       model.solverPool().add<aris::dynamic::ForwardKinematicSolver>();
//   auto& forward_dynamic_solver =
//       model.solverPool().add<aris::dynamic::ForwardDynamicSolver>();

//   model.init();

//   // double ee_xyz_theta[6]{0, 0.8, 0, PI / 4, 0, 0};
//   double ee_xyz_theta[6]{
//       -0.111136585529, 1.1546105246593739, 0, 2.0325816, 0, 0};
//   model.setOutputPos(ee_xyz_theta);
//   if (forward_kinematic_solver.kinPos()) throw std::runtime_error("failed!");

//   // [Inverse_Velocity]
//   // �������ٶȷ��⣬��������ĩ�˵����ٶȺͽ��ٶ�
//   double ee_velocity[6]{-0.851312718725369, -2.06524430307223, 0, 0, 0,
//                         0.0719533168755668};

//   end_effector.setMva(ee_velocity);

//   // ���
//   if (inverse_kinematic_solver.kinVel())
//     throw std::runtime_error("kinematic velocity failed");

//   double ee_acc[6]{-8.30059481465614, -12.4859446875931, 0, 0, 0,
//                    -14.191413340544};
//   end_effector.setMaa(ee_acc);

//   if (model.inverseKinematicsAcc())
//     throw std::runtime_error("inverse kinematic acc failed");

//   std::cout << " Input Accelerate : " << motion2.ma() << " " << motion3.ma()
//             << " " << std::endl;
}

GTEST_TEST(Rokea_config, rokea_config_test) {

}