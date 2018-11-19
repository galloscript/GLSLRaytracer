
#define MARCHING_STEPS 250

vec3 rotate(const in vec3 v, const in vec3 k, const in float theta)
{
    const float cos_theta = cos(theta);
    const float sin_theta = sin(theta);

    vec3 rotated = (v * cos_theta) + (cross(k, v) * sin_theta) + (k * dot(k, v)) * (1 - cos_theta);

    return rotated;
}

float sdSphere( const in vec3 p, const in float s )
{
  return length(p) - s;
}

float sdTorus( const in vec3 p, const in float r0, const in float r1)
{
  //p = m * p;
  vec3 q = vec3(length(vec3(p.x, p.z, 0.f)) - r0, p.y, 0.f);
  return length(q) - r1;
}

float udRoundBox( const in vec3 p, const in vec3 b, const in float r )
{
  //p = m * p; 
  return length(max(abs(p)-b, vec3(0.0f)))-r;
}

// power smooth min (k = 8);
float smin( float a, float b, float k )
{
    a = pow( a, k ); b = pow( b, k );
    return pow( (a*b)/(a+b), 1.0/k );
}

// polynomial smooth min (k = 0.1);
float smin2( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float sdBox( vec3 p, vec3 b)
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec3 rotateY(vec3 p, float theta) 
{
    float c = cos(theta);
    float s = sin(theta);

    return (mat4(
        vec4(c, 0, s, 0),
        vec4(0, 1, 0, 0),
        vec4(-s, 0, c, 0),
        vec4(0, 0, 0, 1)
    ) * vec4(p, 1.0)).xyz;
}

//CHECK ProcMap Function, as there is a commented #define map ProcMap.
const vec3 map(const in vec3 p)
{
    float d = FLT_MAX;
    float b = FLT_MAX;
    uint m = 0;

    //top light
    d = min(d, sdBox(p - vec3(0.f, 3.09f, -10.1f), vec3(0.8, 0.1, 0.8)));
    if (d < b) { m = 6; b = d; }   

    //boxes
    d = min(d, sdBox(rotateY(p - vec3(-1.f, -1.25f, -11.2f), 0.25), vec3(0.8, 1.75, 0.8)));
    if (d < b) { m = 1; b = d; }   
    
    d = min(d, sdBox(rotateY(p - vec3(1.f, -2.2f, -9.f), -0.4), vec3(.85, .85, .85)));
    if (d < b) { m = 0; b = d; }   

    //ball
    //d = min(d, max(sdSphere( p - vec3(1.0, -0.6, -9.f ), 0.75f), -10000.)); //sdSphere( p - vec3(1.0, -0.5, -9.f ), 0.74f)));
    //if (d < b) { m = 4; b = d; }   

    //cornell box walls
    float lBox = max(sdBox(p - vec3(0.f, 0.0f, -10.5f), vec3(3.2)), -sdBox(p - vec3(0.f, 0.0f, -10.f), vec3(3.0f)));
    d = min(d, lBox);
    if (d < b) { m = MAT_CORNELL_BOX; b = d; }

    return vec3(d, float(m), 0);
}

vec3 opMin(vec3 a, vec3 b)
{
    return (a.x < b.x) ? (a) : (b);
}

const vec3 ProcMap(const in vec3 p)
{
    return  opMin
            ( 
                //cornell box walls
                vec3(max(sdBox(p - vec3(0.f, 0.0f, -10.5f), vec3(3.2)), -sdBox(p - vec3(0.f, 0.0f, -10.f), vec3(3.0f))), MAT_CORNELL_BOX, 0),
                opMin
                (
                    //ball 1
                    vec3(sdSphere( p - vec3(1.0, -0.6, -9.f ), 0.75f), 1, 0),
                    opMin
                    (
                        //box 4
                        vec3(sdBox(rotateY(p - vec3(1.f, -2.2f, -9.f), -0.4), vec3(.85, .85, .85)), 0, 0),
                        opMin
                        (
                            //box 2
                            vec3(sdBox(rotateY(p - vec3(-1.f, -1.25f, -11.2f), 0.25), vec3(0.8, 1.75, 0.8)), 0, 0),
                            //light 6
                            vec3(sdBox(p - vec3(0.f, 3.09f, -10.1f), vec3(0.8, 0.1, 0.8)), 6, 0)
                        )
                    )
                )
            );
}


//#define map ProcMap

vec3 calcNormal( const in vec3 pos) 
{
    const float s = 0.5773 * 0.0002;
    const vec3 a = vec3(1, -1, -1) * s;
    const vec3 b = vec3(-1, -1, 1) * s;
    const vec3 c = vec3(-1, 1, -1) * s;
    const vec3 d = vec3(1, 1, 1) * s;
    vec3 n = vec3(a * map(pos + a).x +
            b * map(pos + b).x +
            c * map(pos + c).x +
            d * map(pos + d).x);

    
    //n = (normalize(n) + 1.0f) * 0.5f;

    //TODO: rarest thing i ever seen....
    //n = vec3(1.0 - n.x,  n.y,  1.0 - n.z);

    //n = (normalize(n) * 2.0f) - 1.0f;

    return normalize(vec3( n.x,    n.y,   n.z));
}

vec3 estimateNormal(const in vec3 p) 
{
    const float EPSILON = 0.001f;
    return normalize(vec3(
        map(vec3(p.x + EPSILON, p.y, p.z)).x - map(vec3(p.x - EPSILON, p.y, p.z)).x,
        map(vec3(p.x, p.y + EPSILON, p.z)).x - map(vec3(p.x, p.y - EPSILON, p.z)).x,
        map(vec3(p.x, p.y, p.z  + EPSILON)).x - map(vec3(p.x, p.y, p.z - EPSILON)).x
    ));
}

bool HitRayMarching(const in TRay aRay, const in float aMinDistance, const in float aMaxDistance, inout THitRecord aOutHitRecord)
{
    const vec3 ro = aRay.mOrigin;
    const vec3 rd = aRay.mDirection;

    vec3 h;
    float t = 0.001f;
    bool lFirstWasInside = false;
    float prevDist = 1000.f;
    int greaterTimes = 0;
    for(int i = 0; i < MARCHING_STEPS; ++i)
    {
        h = map((ro + rd * t));

        /*if(i == 0 && h.x < 0.001f)
        {
            //if first was inside wi will consider it is a refraction ray
            h.x = 0.0019f;
            lFirstWasInside = true;
        }
        else if(lFirstWasInside && h.x < 0.001f)
        {
            h.x = abs(h.x);
        }*/
        t += h.x;
        if(h.x < 0.001f || t > aMaxDistance){ break; }


                //optimization
        /*if(h.x > 0)
        {
            if(h.x > prevDist) //h.x > 0  && h.x < 0.01 &&
            {
                h.x *= 1.5;
                greaterTimes++;
            }
            else
            {
                greaterTimes = 0;
            }

            if(greaterTimes >= 2)
            {
                break;
            }
        }*/

        prevDist = h.x;
        
    }

    vec3 endPoint = ro + rd * t;

    if ( h.x < 0.001f && t > 0.0015f && t < aMaxDistance) 
    {
        aOutHitRecord.mDistance = t;
        aOutHitRecord.mMaterialId = uint(h.y);
        aOutHitRecord.mNormal = calcNormal(endPoint);
        aOutHitRecord.mPoint = endPoint;

        return true;
    }

    return false;
}
