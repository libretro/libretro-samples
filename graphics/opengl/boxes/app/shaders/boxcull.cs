layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

layout(binding = GLOBAL_VERTEX_DATA) uniform GlobalVertexData
{
   mat4 vp;
   mat4 view;
   mat4 view_nt;
   mat4 proj;
   mat4 inv_vp;
   mat4 inv_view;
   mat4 inv_view_nt;
   mat4 inv_proj;
   vec4 camera_pos;
   vec4 camera_vel;
   vec4 resolution;
   vec4 frustum[6];
   float delta_time;
} global_vert;

layout(binding = 0, offset = 4) uniform atomic_uint lod0_cnt; // Outputs to instance variable.
layout(binding = 0, offset = 24) uniform atomic_uint lod1_cnt;
layout(binding = 0, offset = 40) uniform atomic_uint lod2_cnt; // not 44 since we're using point sprites here with glDrawArraysIndirect.

struct Point
{
   vec4 pos;
   vec4 vel;
};

layout(binding = 0) buffer SourceData
{
   Point points[];
} source_data;

layout(binding = 1) buffer DestData0
{
   writeonly vec4 pos[];
} culled0;

layout(binding = 2) buffer DestData1
{
   writeonly vec4 pos[];
} culled1;

layout(binding = 3) buffer DestData2
{
   writeonly vec4 pos[];
} culled2;

uint get_invocation()
{
   uint work_group = gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z + gl_WorkGroupID.y * gl_NumWorkGroups.z + gl_WorkGroupID.z;
   return work_group * gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z + gl_LocalInvocationIndex;
}

void main()
{
   uint invocation = get_invocation();
   vec4 point = source_data.points[invocation].pos;
   vec4 vel = source_data.points[invocation].vel;

   // "Physics" :D
   vec3 dist = point.xyz - global_vert.camera_pos.xyz;
   float dist_len_sq = dot(dist, dist);

   vec3 accel_neg = 20000.0 * -normalize(dist) / (dot(dist, dist) + 0.001);
   point.xyz += global_vert.delta_time * vel.xyz;
   vel.xyz += global_vert.delta_time * accel_neg;

   vec3 rel_vel = vel.xyz - global_vert.camera_vel.xyz; // Relative velocity to camera.
   if (dist_len_sq < 10.0 && dot(rel_vel, dist) < 0.0)
      vel.xyz = reflect(rel_vel, normalize(dist)) + global_vert.camera_vel.xyz; // Bounce factor

   source_data.points[invocation].pos = point;
   source_data.points[invocation].vel = vel;

   vec4 pos = vec4(point.xyz, 1.0);

   // Frustum cull and create instance draw lists.

   float depth = dot(pos, global_vert.frustum[0]);
   if (depth < -point.w) // Culled
      return;

   for (int i = 1; i < 6; i++)
      if (dot(pos, global_vert.frustum[i]) < -point.w) // Culled
         return;

   if (depth > 500.0) // LOD2
   {
      uint counter = atomicCounterIncrement(lod2_cnt);
      culled2.pos[counter] = point;
   }
   else if (depth > 100.0) // LOD1
   {
      uint counter = atomicCounterIncrement(lod1_cnt);
      culled1.pos[counter] = point;
   }
   else // LOD0
   {
      uint counter = atomicCounterIncrement(lod0_cnt);
      culled0.pos[counter] = point;
   }
}

