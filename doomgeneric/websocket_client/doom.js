import * as twgl from 'twgl.js';

let frameTime = 0, prev = performance.now(), curr;
let fpsDiv;

const vshader = `
  attribute vec2 a_position;
  varying vec2 v_texCoord;

  void main() {
     gl_Position = vec4(a_position, 0.0, 1.0);
     v_texCoord = a_position*.5+.5;
     v_texCoord.y = 1.-v_texCoord.y;
  }
`;

const fshader = `
  precision mediump float;

  uniform sampler2D u_image;
  varying vec2 v_texCoord;

  void main() {
     gl_FragColor = texture2D(u_image, v_texCoord);
  }
`;

class Renderer {
  constructor(canvas) {
    let gl;
    try {
      gl = canvas.getContext(
        "webgl",
        { preserveDrawingBuffer: true }
      );
      this.gl = gl;
      gl.viewportWidth = canvas.width;
      gl.viewportHeight = canvas.height;
    } catch (e) {
      if (!this.gl) {
        alert("Could not initialise WebGL, sorry :-(");
      }
    }

    this.program = twgl.createProgram(gl, [vshader, fshader]);
    gl.useProgram(this.program);

    this.positionLocation = gl.getAttribLocation(this.program, "a_position");

    const verts = [
      -1.0, -1.0,
      1.0, -1.0,
      -1.0, 1.0,
      1.0, -1.0,
      1.0, 1.0,
      -1.0, 1.0,
    ];
    this.vertBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(verts), gl.STATIC_DRAW);

    gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(this.positionLocation);

    const tex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, tex);

    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGBA,
      1,
      1,
      0,
      gl.RGBA,
      gl.UNSIGNED_BYTE,
      new Uint8Array([0, 0, 255, 1])
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    gl.clearColor(1.0,0.0,0.0,1.0);
  }

  render = (data) => {
    const gl = this.gl;
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGBA,
      640,
      400,
      0,
      gl.RGBA,
      gl.UNSIGNED_BYTE,
      data,
    );

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.useProgram(this.program);

    // Turn on the vertex attribute
    gl.enableVertexAttribArray(this.positionLocation);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertBuffer);
    gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, 0, 0);

    // Draw the rectangle
    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }
}

function init() {
  fpsDiv = document.getElementById('fps');
  canvas = document.getElementById('canvas');
  const renderer = new Renderer(canvas);

  const ws = new WebSocket('ws://localhost:5000', 'grafana-doom');
  ws.binaryType = 'arraybuffer';

  ws.addEventListener('message', ({ data }) => {
    curr = performance.now();
    frameTime = (curr - prev);
    prev = curr;
    if (renderer) {
      requestAnimationFrame(() => renderer.render(new Uint8Array(data)));
      displayFPS(data);
    }
  })

  function displayFPS(){
    fpsDiv.innerText = `FPS: ${Math.round(1000 / frameTime)}`;
  }
}


window.onload = () => {
  init();
}
