import React from 'react'
// import { Canvas, useLoader } from '@react-three/fiber'
// import { ConfigConsumer } from "antd/lib/config-provider";
// import { ConfigConsumerProps } from "antd/lib/config-provider/context";
// 
// 
// import { STLLoader } from "three/examples/jsm/loaders/STLLoader";
// import { BackSide, Plane, Vector3 } from 'three';
// 
// interface CellProps {
//   prefixCls: string
// }
// 
// function Light(props: any) {
//   return (
//     <mesh position={props.position} scale={props.scale}>
//       <boxBufferGeometry />
//       <meshLambertMaterial color={0x000000} emissive={0xffffff} emissiveIntensity={10} />
//     </mesh>
//   )
// }
// 
// function MainLight() {
//   return (
//     <pointLight position={[1, 1, 1]} color={0xffffff} intensity={5} distance={0} decay={0.2} />
//   );
// }
// 
// function Room() {
//   return (
//     <mesh scale={10}>
//       <boxBufferGeometry />
//       <meshStandardMaterial metalness={0} side={BackSide} />
//     </mesh>
//   );
// }
// 
// function STLObject(props: any) {
//   const stl = useLoader(STLLoader, props.path,);
//   return <primitive object={stl} scale={1} />
// }
// 
// function renderThree(context: ConfigConsumerProps, customizePrefixCls: string) {
//   const prefixCls = context.getPrefixCls("display3d", customizePrefixCls);
//   return (
//     <div className={`${prefixCls}`}>
//       <Canvas camera={{ fov: 45, near: 0.1, far: 100, position: [-1, -1, 0.1], lookAt: () => new Vector3(0, 0, 0) }}>
//         <color attach="background" args={[0x000000]} />
//         <fog attach="fog" color={0x000000} near={2} far={10} />
//         <hemisphereLight position={[0, 0, 200]} />
//         <directionalLight position={[0, 10, 10]} />
//         <mesh receiveShadow>
//           <planeBufferGeometry args={[2000, 2000]} />
//           <meshPhongMaterial color={0x111111} depthWrite={false} />
//           <gridHelper rotation={[Math.PI / 2, 0, 0]}>
//             <material attach="material" opacity={0.1} depthWrite={false} transparent />
//           </gridHelper>
// 
//         </mesh>
//       </Canvas>
//     </div>
//   )
// }
// 
// function Display3d(props: CellProps) {
//   return <ConfigConsumer>{context => renderThree(context, props.prefixCls)}</ConfigConsumer>
// }