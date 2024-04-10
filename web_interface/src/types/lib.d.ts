export interface CellConfig {
  name: string;
  i: string;
  x: number;
  y: number;
  width: number;
  height: number;
  options: string;
  optionsJson: Object;
}

interface DashboardConfig {
  i: string;
  editable: boolean;
  cells: CellConfig[];
}

interface FetchError {
  error: string;
}

interface FetchResponse<T> {
  response: T;
}

export interface GeometryBase {
  geometry_id: number;
}

export interface GeometryOnPart extends GeometryBase {
  part_id: number;
  relative_to_part: boolean;
}


export interface SphereGeomtry extends GeometryOnPart{
  shape_type: string;
  radius: number;
}

export interface BoxGeomtry extends GeometryOnPart{
  shape_type: string;
  length: number;
  width: number;
  height: number;
}
// declare global {
//   interface window { __REDUX_DEVTOOLS_EXTENSION_COMPOSE__: any }
// }
