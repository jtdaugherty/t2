#define STACK_DEPTH 20
#define TRACE_DEPTH 10

#define MAX_PLANES 10
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
#define MAX_MATERIALS 10

#define EPSILON 0.001f

struct Ray
{
	float4 origin;
	float4 dir;
};

struct Sphere
{
	uint  material;
	float4 center;
	float sqradius;
};

struct Plane
{
	uint material;
	float4 normal;
	float dist;
};

struct Light
{
	uint  material;
	float4 center;
};

struct Material
{
	float refl;
	float refr;
	float diff;
	float spec;
	float4 amb;
};

struct Scene
{
	struct Plane planes[MAX_PLANES];
	struct Sphere spheres[MAX_SPHERES];
	struct Light  lights[MAX_LIGHTS];
	struct Material materials[MAX_MATERIALS];

	uint numPlanes;
	uint numSpheres;
	uint numLights;
	uint numMaterials;
};

void buildscene(struct Scene *s)
{
	s->spheres[s->numSpheres].center = (float4)(1, -0.8f, 3, 0);
	s->spheres[s->numSpheres].sqradius = 2.5f * 2.5f;
	s->spheres[s->numSpheres].material = 0;
	s->numSpheres++;

	s->spheres[s->numSpheres].center = (float4)(1, -0.5f, 7, 0);
	s->spheres[s->numSpheres].sqradius = 2 * 2;
	s->spheres[s->numSpheres].material = 5;
	s->numSpheres++;

	s->spheres[s->numSpheres].center = (float4)(-5.5f, -0.5f, 7, 0);
	s->spheres[s->numSpheres].sqradius = 2 * 2;
	s->spheres[s->numSpheres].material = 1;
	s->numSpheres++;

	s->planes[s->numPlanes].normal = (float4)(0, 1, 0, 0);
	s->planes[s->numPlanes].dist = -4.4f;
	s->planes[s->numPlanes].material = 2;
	s->numPlanes++;

	s->materials[0].refl = 0;
	s->materials[0].refr = 1.1f;
	s->materials[0].spec = 127;
	s->materials[0].amb  = (float4)(0.7f, 0.7f, 0.7f, 0);
	s->materials[0].diff = 0.1f;
	s->numMaterials = 1;

	s->materials[1].refl = 1;
	s->materials[1].refr = 0;
	s->materials[1].spec = 120;
	s->materials[1].amb  = (float4)(0.7f, 0.7f, 1.0f, 0);
	s->materials[1].diff = 0.1f;
	s->numMaterials++;

	s->materials[2].refl = 0;
	s->materials[2].refr = 0;
	s->materials[2].spec = 120;
	s->materials[2].amb  = (float4)(0.4f, 0.3f, 0.3f, 0);
	s->materials[2].diff = 1;
	s->numMaterials++;

	s->materials[3].refl = 0;
	s->materials[3].refr = 0;
	s->materials[3].spec = 120;
	s->materials[3].amb  = (float4)(0.4f, 0.4f, 0.4f, 0);
	s->materials[3].diff = 1;
	s->numMaterials++;

	s->materials[4].refl = 0;
	s->materials[4].refr = 0;
	s->materials[4].spec = 120;
	s->materials[4].amb  = (float4)(0.6f, 0.6f, 0.8f, 0);
	s->materials[4].diff = 1;
	s->numMaterials++;

	s->materials[5].refl = 0;
	s->materials[5].refr = 0;
	s->materials[5].spec = 120.f;
	s->materials[5].amb  = (float4)(0.3f, 1.0f, 0.4f, 0);
	s->materials[5].diff = 1;
	s->numMaterials++;

	s->lights[0].center = (float4)(0, 5, 5, 0);
	s->lights[0].material = 3;
	s->numLights++;

	s->lights[1].center = (float4)(2, 5, 1, 0);
	s->lights[1].material = 4;
	s->numLights++;
}

float4 planenormal(struct Plane *p)
{
	return p->normal;
}

int planeintersect(struct Plane *p, struct Ray *r, float *dist)
{
	float d = (-dot(p->normal, r->origin) + p->dist) / dot(p->normal, r->dir);

	if(d > 0 && d < *dist)
	{
		*dist = d; 
		return 1;
	}

	return 0;
}

float4 spherenormal(struct Sphere *s, float4 poi)
{
	return normalize(poi - s->center);
}

int sphereintersect(struct Sphere *s, struct Ray *r, float *dist)
{
	float4 v = r->origin - s->center;
	float b = -dot(v, r->dir);
	float det = (b * b) - dot(v, v) + s->sqradius;

	if(det > 0)
	{
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;

		if(i2 > 0)
		{
			if(i1 < 0)
			{
				if(i2 < *dist) { *dist = i2; return -1; }
			}
			else
			{
				if(i1 < *dist) { *dist = i1; return 1; }
			}
		}
	}

	return 0;
}

float4 reflect(float4 A, float4 B)
{
	return B - ((float4)2) * (float4)dot(A, B) * A;
}

struct RayStack
{
	struct Ray r[STACK_DEPTH];
	int depth[STACK_DEPTH];
	float refr[STACK_DEPTH];
	int top;
};

void push(struct RayStack *s, struct Ray *r, float refr, int depth)
{
	if(s->top < STACK_DEPTH)
	{
		s->r[s->top].dir = r->dir;
		s->r[s->top].origin = r->origin;
		s->refr[s->top] = refr;
		s->depth[s->top] = depth;
		s->top++;
	}
}

struct IntersectionResult
{
	int result;
	float4 normal;
	float4 position;
	float distance;
	struct Material *material;
};

int findintersection(struct Scene *s, struct Ray *r, struct IntersectionResult *intersection)
{
	int result = 0;
	float dist = MAXFLOAT;
	int sphere = -1;
	int plane  = -1;

	for(uint i = 0; i < s->numSpheres; i++)
	{
		int res = sphereintersect(&s->spheres[i], r, &dist);
		if(res) result = res, sphere = i;
		if(res && !intersection) return res;
	}

	for(uint i = 0; i < s->numPlanes; i++)
	{
		int res = planeintersect(&s->planes[i], r, &dist);
		if(res) result = res, plane = i;
		if(res && !intersection) return res;
	}

	if(intersection)
	{
		intersection->result = result;
		intersection->distance = dist;
		intersection->position = r->origin + r->dir * dist;

		if(plane != -1)
		{
			intersection->normal = planenormal(&s->planes[plane]);
			intersection->material = &s->materials[s->planes[plane].material];
		}
		else if(sphere != -1)
		{
			intersection->normal = spherenormal(&s->spheres[sphere], intersection->position);
			intersection->material = &s->materials[s->spheres[sphere].material];
		}
	}

	return result;
}

float shadowray(struct Scene *s, float4 L, float4 P)
{
	float t = length(L);
	L *= 1.f / t;

	struct Ray light;
	light.origin = P + L * EPSILON;
	light.dir = L;

	return findintersection(s, &light, 0) ? 0.f : 1.f;
}

float4 raytrace(struct Scene *s, struct RayStack *stack, struct Ray *r, float refr, int depth)
{
	float4 color = (float4)(0, 0, 0, 0);

	if(depth > TRACE_DEPTH) return color;

	struct IntersectionResult intersection;
	int result = findintersection(s, r, &intersection);

	if(result == 0) return color;

	struct Material *m  = intersection.material;
	float4 P = intersection.position;
	float4 N = intersection.normal;

	for(uint i = 0; i < s->numLights; i++)
	{
		struct Material *lm = &s->materials[s->lights[i].material];
		float4 L = s->lights[i].center - P;

		float shade = shadowray(s, L, P);

		L = normalize(L);

		float angle = fmax(0.f, dot(N, L)) * shade;
		float s = dot(r->dir, reflect(N, L)) * shade;

		color += angle * m->diff * m->amb * lm->amb 
			+ powr(fmax(0.f, s), m->spec) * lm->amb;
	}

	if(m->refl > 0)
	{
		float4 refl = reflect(N, r->dir);

		struct Ray R;
		R.origin = P + refl * EPSILON;
		R.dir = refl;

		push(stack, &R, refr, depth + 1);
	}

	if(m->refr > 0)
	{
		float4 refrN = N * (float)result;
		float n = refr / m->refr;
		float cos_i = -dot(refrN, r->dir);
		float cos_t2 = 1.f - n * n * (1 - cos_i * cos_i);

		if(cos_t2 > 0)
		{
			float4 T = n * r->dir + (n * cos_i - sqrt(cos_t2)) * refrN;

			struct Ray R;
			R.origin = P + T * EPSILON;
			R.dir = T;

			push(stack, &R, m->refr, depth + 1);
		}
	}

	return color;
}

float4 recursivetrace(struct Scene *s, struct Ray *r)
{
	struct RayStack stack;
	stack.top = 0;
	push(&stack, r, 1.f, 0);

	float4 c = (float4)(0, 0, 0, 0);

	while(stack.top > 0)
	{
		stack.top--;
		c += raytrace(s, &stack, &stack.r[stack.top], stack.refr[stack.top], stack.depth[stack.top]);
	}

	return c;
}

__kernel void raytracer(__write_only image2d_t output, uint width, uint height, float z)
{
	int2 pos = (int2)(get_global_id(0), get_global_id(1));
	float2 screen = (float2)(
		pos.x / (float)width * 8.f - 4,
		pos.y / (float)height * 6.f - 3
	);

	struct Ray r;

	r.origin = (float4)(0, 0, z, 0);
	r.dir    = normalize((float4)(screen.x, screen.y, 0, 0) - r.origin);

	struct Scene s;
	s.numSpheres = s.numLights = s.numMaterials = s.numPlanes = 0;
	buildscene(&s);

	write_imagef(output, pos, recursivetrace(&s, &r));
}
