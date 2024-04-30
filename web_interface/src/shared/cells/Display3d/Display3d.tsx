import React, {
  Suspense,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
} from "react";
import {
  Canvas,
  useLoader,
  Vector3 as Vector3Type,
  Quaternion as Vector4Type,
} from "@react-three/fiber";
import {
  ConfigConsumerProps,
  ConfigContext,
} from "antd/lib/config-provider/context";
import { RedoOutlined } from "@ant-design/icons";
import { CameraControls } from "@react-three/drei";

import { STLLoader } from "three/examples/jsm/loaders/STLLoader";
import { FBXLoader } from "three/examples/jsm/loaders/FBXLoader";
// import { BackSide, PerspectiveCamera, Plane, Vector3 } from 'three';
import {
  getGeometryPm,
  getGeometryPool,
  getViewerConfig,
} from "../../../state/selectors";
import {
  display3dInitRequest,
  sendCmdSilence,
  // deleteResultByChannel
} from "../../../state/actions";
import { shallowEqual, useDispatch, useSelector } from "react-redux";
import { RootState } from "~/state/reducers";
import { Display3dConfig } from "./types";
import { withSize } from "react-sizeme";
import {
  BufferGeometry,
  Group,
  LoadingManager,
  Material,
  Matrix4,
  Mesh,
  MeshStandardMaterial,
  NormalBufferAttributes,
  Object3DEventMap,
  Quaternion,
  Vector3,
} from "three";

import {
  Matrix4Array,
  PositionType,
  QuaternionType,
  SireBoxGeometry,
  SireGeometry,
  SireMeshGeometry,
  SireSphereGeometry,
} from "~/types/lib";

interface CellProps {
  prefixCls: string;
  size: { width: number; height: number };
}
const default_material: MeshStandardMaterial = new MeshStandardMaterial({
  color: "rgb(255,255,255)",
  roughness: 0.1,
  metalness: 0.1,
});

const material_odd_part: MeshStandardMaterial = new MeshStandardMaterial({
  color: "rgb(6,96,255)",
  roughness: 0.1,
  metalness: 0.1,
});

const material_even_part: MeshStandardMaterial = new MeshStandardMaterial({
  color: "rgb(212,212,212)",
  roughness: 0.1,
  metalness: 0.1,
});

const default_scale: number = 1;

const default_position: PositionType = [0, 0, 0];
const default_quaternion: QuaternionType = [0, 0, 0, 1];

interface ModelProps {
  path: string;
  material?: Material;
  scale?: number;
  position?: Vector3Type;
  quaternion?: Vector4Type;
}

const FBXModel = ({
  path,
  material = default_material,
  scale = default_scale,
  position = default_position,
  quaternion = default_quaternion,
}: ModelProps) => {
  const fbx: Group<Object3DEventMap> = useMemo(useLoader(FBXLoader, path), [
    path,
  ]);
  fbx.children.forEach((mesh) => {
    (mesh as Mesh).material = material;
  });

  return (
    <mesh position={position} quaternion={quaternion} scale={scale}>
      <primitive object={fbx} attach="geometry" />
    </mesh>
  );
};

const STLModel = ({
  path,
  material = default_material,
  scale = default_scale,
  position = default_position,
  quaternion = default_quaternion,
}: ModelProps) => {
  const stl: BufferGeometry<NormalBufferAttributes> = useMemo(
    useLoader(STLLoader, path),
    [path]
  );
  return (
    <mesh
      material={material}
      position={position}
      quaternion={quaternion}
      scale={scale}
    >
      <primitive object={stl} attach="geometry" />
    </mesh>
  );
};

interface RobotMeshProps {
  poses?: number[][];
  scales?: number[];
}

// TODO: init_pm没有任何作用，建议放在内部处理，感觉发送display3d_init没有必要保存状态在全局，放在Display3d component中就可以了
function RobotMesh({ poses, scales }: RobotMeshProps) {
  const geometry_pool = useSelector<RootState, SireGeometry[] | null>(
    getGeometryPool,
    shallowEqual
  );
  const geometry_pm_data = useSelector<RootState, Matrix4Array[] | null>(
    getGeometryPm,
    shallowEqual
  );
  const evaluate_pose = useCallback(
    (
      geometry: SireGeometry,
      pose?: number[],
      geometry_pm?: Matrix4Array
    ): [PositionType | Vector3, Quaternion | QuaternionType] => {
      if (!pose) {
        return [default_position, default_quaternion];
      }
      if (!geometry.relative_to_part || !geometry_pm) {
        // directly update
        return [
          [pose[0], pose[1], pose[2]],
          [pose[3], pose[4], pose[5], pose[6]],
        ];
      } else {
        let current_position = new Vector3(pose[0], pose[1], pose[2]);
        let current_rotation = new Quaternion(
          pose[3],
          pose[4],
          pose[5],
          pose[6]
        );
        let current_scale = new Vector3(1, 1, 1);
        let current_pm = new Matrix4().compose(
          current_position,
          current_rotation,
          current_scale
        );
        let geo_pm = new Matrix4().set(...geometry_pm);
        current_pm.multiply(geo_pm);
        current_position.setFromMatrixPosition(current_pm);
        current_rotation.setFromRotationMatrix(current_pm);
        current_scale.setFromMatrixScale(current_pm);
        return [current_position, current_rotation];
      }
    },
    []
  );

  const processed_pose = useMemo(
    () =>
      geometry_pool?.map((geometry: SireGeometry, i) => {
        return evaluate_pose(geometry, poses?.at(i), geometry_pm_data?.at(i));
      }),
    [poses]
  );
  return (
    <>
      {geometry_pool?.map((geometry, i) => {
        let [position, quaternion]: [
          PositionType | Vector3Type,
          QuaternionType | Quaternion
        ] = [default_position, default_quaternion];
        if (processed_pose?.at(i)) {
          [position, quaternion] = processed_pose[i];
        }
        const material: MeshStandardMaterial =
          i % 2 ? material_odd_part : material_even_part;
        switch (geometry.shape_type) {
          case "sphere":
            return (
              <mesh
                material={material}
                position={position}
                quaternion={quaternion}
              >
                <sphereGeometry
                  args={[(geometry as SireSphereGeometry).radius]}
                />
              </mesh>
            );
          case "box":
            return (
              <mesh
                material={material}
                position={position}
                quaternion={quaternion}
              >
                <boxGeometry
                  args={[
                    (geometry as SireBoxGeometry).length,
                    (geometry as SireBoxGeometry).width,
                    (geometry as SireBoxGeometry).height,
                  ]}
                />
              </mesh>
            );
          case "mesh":
            return (
              <FBXModel
                path={(geometry as SireMeshGeometry).resource_path}
                position={position}
                quaternion={quaternion}
                material={material}
                scale={scales?.at(i)}
              />
            );
          default:
            console.log("unrecoginized geometry!");
            break;
        }
        return;
      })}
    </>
  );
}

const DefaultConfig: Display3dConfig = {
  frame_rate: 30,
};

function Loading({ prefixCls }: { prefixCls: string }) {
  return <div className={`${prefixCls}-loading`}>载入中...</div>;
}

const Display3d = (props: CellProps) => {
  const dispatch = useDispatch();
  const config = useSelector<RootState, Display3dConfig>(
    (state) => getViewerConfig(state, DefaultConfig),
    shallowEqual
  );
  const update_location_count = useRef<number>(0);
  const prev_update_location_time = useRef<number>(performance.now());
  const [location_update_per_second, set_LUPS] = useState<number>(0);
  const location_update_timer = useRef<number | null>(null);
  const [pose, setPose] = useState<number[][]>();
  const camera_control = useRef<CameraControls | null>(null);

  const evalute_location_update_per_second = useCallback(() => {
    update_location_count.current++;
    const current_time: number = performance.now();

    if (current_time >= prev_update_location_time.current + 500) {
      set_LUPS(
        Math.round(
          (update_location_count.current * 1000) /
            (current_time - prev_update_location_time.current)
        )
      );
      update_location_count.current = 0;
      prev_update_location_time.current = current_time;
    }
  }, []);

  // execute when component initialize.
  useEffect(() => {
    console.log("send display3d init");
    // this.props.deleteResultByChannel('xml')
    dispatch(display3dInitRequest());
    // this.props.loadConfigXml();
    location_update_timer.current = setInterval(() => {
      dispatch(
        sendCmdSilence("get --part_pq", (msg: any) => {
          if (msg && msg.jsData && msg.jsData.return_code === 0) {
            setPose(msg.jsData.part_pq);
            evalute_location_update_per_second();
          }
        })
      );
    }, 1000.0 / config.frame_rate);

    return () => {
      if (location_update_timer.current) {
        clearInterval(location_update_timer.current);
        location_update_timer.current = null;
      }
    };
  }, []);
  const { getPrefixCls, rootPrefixCls } =
    useContext<ConfigConsumerProps>(ConfigContext);
  const prefixCls = getPrefixCls("display3d", rootPrefixCls);
  return (
    <div className={`${prefixCls}`}>
      <Suspense fallback={<Loading prefixCls={prefixCls} />}>
        <div className={`${prefixCls}-three`}>
          <RedoOutlined
            onClick={() => {
              camera_control.current?.reset(true);
            }}
          />
          <div className={`${prefixCls}-three-info`}>
            {/* <div className={`${prefixCls}-three-info-fps`}>fps: {this.state.fps}</div> */}
            <div className={`${prefixCls}-three-info-lups`}>
              LUPS: {location_update_per_second}
            </div>
          </div>
          <Canvas
            camera={{
              fov: 50,
              near: 0.1,
              //far: 10,
              //position: [-1, -1, 0.1],
              //lookAt: () => new Vector3(0, 0, 0),
            }}
          >
            <CameraControls ref={camera_control} />
            <color attach="background" args={[0x000000]} />
            {/* <fog attach="fog" color={0x000000} near={2} far={10} /> */}
            <hemisphereLight
              position={[0, 0, 200]}
              color={0xffffff}
              groundColor={0x444444}
            />
            <directionalLight
              position={[0, 10, 10]}
              color={0xffffff}
              intensity={2}
            />
            {/* <mesh receiveShadow={true}>
              <planeGeometry attach="geometry" args={[2000, 2000]} />
              <meshPhongMaterial attach="material" color={0x111111} opacity={0}/>
            </mesh> */}
            <gridHelper rotation={[Math.PI / 2, 0, 0]}>
              <lineBasicMaterial
                opacity={0.3}
                depthWrite={false}
                transparent={true}
              />
            </gridHelper>
            <RobotMesh poses={pose} />
            <ambientLight color={0x404040} intensity={5} />
            <hemisphereLight
              position={[0, 0, 200]}
              color={0xffffff}
              groundColor={0x444444}
            />
            <directionalLight
              position={[0, 10, 10]}
              color={0xffffff}
              intensity={2}
            />
          </Canvas>
        </div>
      </Suspense>
    </div>
  );
};

Display3d.NAME = "仿真可视化";
const sizedDisplay = withSize({ monitorHeight: true, refreshRate: 30 })(
  Display3d
);
export default sizedDisplay;
