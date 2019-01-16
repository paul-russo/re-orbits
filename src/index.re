open Reprocessing;

/* Constants */
let g = 6.673e-11; /* N∙m^2/kg^2 */
let earthMass = 5.98e24;
let earthRadius = 6.3781e6;

/* Config */
let width = 600.0;
let height = 600.0;

let cx = width /. 2.0;
let cy = height /. 2.0;

type body = {
  v: float, /* orbital velocity, m/s */
  av: float, /* angular velocity, rad/s */
  mass: float, /* kg */
  r: float, /* distance from center, m */
  rad: float, /* radius of object, m */
  color: colorT,
  a: float, /* angle relative to the center point (clockwise, 3-o-clock is 2pi), radians */
};

type state = {
  scale: float, /* meters per pixel */
  timescale: float, /* seconds per frame */
  center: body,
  bodies: list(body),
};

let computeV = (m: float, r: float) => {
  sqrt((g *. m) /. r)
}

let setup = (env: glEnvT) => {
  Env.size(~width=int_of_float(width), ~height=int_of_float(height), env);

  {
    scale: 150000.0,
    timescale: 50.0,
    center: {
      v: 0.0,
      av: 0.0,
      mass: earthMass,
      r: 0.0,
      rad: earthRadius,
      color: Constants.blue,
      a: 0.0,
    },
    bodies: [],
  }
};


let draw = (state: state, env: glEnvT) => {
  let { center, scale, bodies, timescale } = state;

  /* Draw background */
  Draw.fill(Constants.black, env);
  Draw.rect(~pos=(0, 0), ~width=int_of_float(width), ~height=int_of_float(height), env);

  /* Draw center body */
  let radpx = int_of_float(center.rad /. scale);
  Draw.fill(center.color, env);
  Draw.ellipse(~center=(int_of_float(cx), int_of_float(cy)), ~radx=radpx, ~rady=radpx, env);

  /* Draw orbiting bodies */
  let updatedBodies = List.map((body) => {
    let x = (cx *. scale) +. body.r *. cos(body.a);
    let y = (cy *. scale) +. body.r *. sin(body.a);

    let radpx = int_of_float(body.rad /. scale);
    let xpx = int_of_float(x /. scale);
    let ypx = int_of_float(y /. scale);

    Draw.fill(body.color, env);
    Draw.ellipse(~center=(xpx, ypx), ~radx=radpx, ~rady=radpx, env);

    {
      ...body,
      /* Update the angle based on angular velocity/timescale */
      a: body.a +. (body.av *. timescale),
    }
  })(bodies);

  {
    ...state,
    bodies: updatedBodies,
  }
};

let addSatellite = (state: state, x: int, y: int) => {
  let num = List.length(state.bodies) + 1;

  let a = cx -. float_of_int(x);
  let b = cy -. float_of_int(y);
  let c = sqrt((a ** 2.0) +. (b ** 2.0));

  let r = c *. state.scale;

  if (r > state.center.rad) {
    let v = computeV(state.center.mass, r);
    let a = (atan2(b, a) -. atan2(0.0, cx)) +. Constants.pi;

    /* Angular velocity is v/r */
    let av = v /. r;

    Printf.printf("Added body #%d. Radius: %.0fm, Height: %.0fm, Angle: %.4frad, Velocity: %.0fm/s", num, r, r -. state.center.rad, a, v);
    print_newline();

    {
      ...state,
      bodies: [
        {
          v,
          a,
          r,
          av,
          mass: 0.0,
          rad: earthRadius /. 4.0,
          color: Constants.red,
        },
        ...state.bodies,
      ]
    }
  } else {
    Printf.printf("Couldn't add body #%d: orbit radius of %.0fm is too low!", num, r);
    print_newline();

    state
  }
};

let mouseDown = (state: state, env: glEnvT) => {
  let (x, y) = Env.mouse(env);

  addSatellite(state, x, y)
};

run(~setup, ~draw, ~mouseDown, ());
