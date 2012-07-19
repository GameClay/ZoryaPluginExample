require 'render.ShaderManager'

ShaderManager.declareshader('Vertex.GL3', [[
   in vec3 InPosition;

   out vec4 color0;
   out float cqt_val;

   uniform vec3 cqtColors[12];

   void main()
   {
      // InPosition.xyz = {i, bins_per_octave, num_octaves}
      float interp = InPosition.x;
      cqt_val = InPosition.y;

      int cidx = int(fract(InPosition.z) * 12.0);

      color0 = vec4(cqtColors[cidx], 1.0);

      gl_Position = vec4(-1.0 + interp * 2.0, -1.0, 0.0, 1.0);
   }
]])

ShaderManager.declareshader('Geometry.GL3', [[
   layout(points) in;
   layout(line_strip, max_vertices = 2) out;

   in vec4 color0[];
   in float cqt_val[];

   out vec4 frag_color0;

   uniform mat4 object_to_screen;

   void main()
   {
      vec4 center = gl_in[0].gl_Position;

      gl_Position = object_to_screen * center;
      frag_color0 = color0[0];
      EmitVertex();

      gl_Position = object_to_screen * (center + vec4(0.0, cqt_val[0], 0.0, 0.0));
      frag_color0 = color0[0];
      EmitVertex();

      EndPrimitive();
   }
]])

ShaderManager.declareshader('Fragment.GL3', [[
   in vec4 frag_color0;

   out vec4 fragout0;

   void main()
   {
      fragout0 = frag_color0;
   }
]])
