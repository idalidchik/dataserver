﻿// math_util.cpp
//
#include "dataserver/spatial/math_util.h"

namespace sdl { namespace db { namespace math_util_ {

// https://en.wikipedia.org/wiki/Point_in_polygon 
// https://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
// Run a semi-infinite ray horizontally (increasing x, fixed y) out from the test point, and count how many edges it crosses. 
// At each crossing, the ray switches between inside and outside. This is called the Jordan curve theorem.

#if SDL_DEBUG > 1
template<class float_>
int pnpoly(int const nvert,
           float_ const * const vertx, 
           float_ const * const verty,
           float_ const testx, 
           float_ const testy,
           float_ const epsilon = 0)
{
  int i, j, c = 0;
  for (i = 0, j = nvert-1; i < nvert; j = i++) {
    if ( ((verty[i]>testy) != (verty[j]>testy)) &&
     ((testx + epsilon) < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
       c = !c;
  }
  return c;
}
#endif

// https://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
// https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
inline bool ray_crossing(point_2D const & test, point_2D const & p1, point_2D const & p2) {
    return ((p1.Y > test.Y) != (p2.Y > test.Y)) &&
        ((test.X + limits::fepsilon) < ((test.Y - p2.Y) * (p1.X - p2.X) / (p1.Y - p2.Y) + p2.X));
}

} // math_util_

using namespace math_util_;

//https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

bool math_util::get_line_intersect(point_2D & result,
    point_2D const & _1, point_2D const & _2, // line1
    point_2D const & _3, point_2D const & _4) // line2
{
    double const denominator  = (_1.X - _2.X) * (_3.Y - _4.Y) - (_1.Y - _2.Y) * (_3.X - _4.X);
    if (fzero(denominator )) {
        SDL_ASSERT(0);
        return false;
    }
    double const A = _1.X * _2.Y - _1.Y * _2.X;
    double const B = _3.X * _4.Y - _3.Y * _4.X;
    result.X = (A * (_3.X - _4.X) - (_1.X - _2.X) * B) / denominator;
    result.Y = (A * (_3.Y - _4.Y) - (_1.Y - _2.Y) * B) / denominator;
    return true;
}

#if 0
bool math_util::line_intersect(
                            point_2D const & a, point_2D const & b,   // line1 (a,b)
                            point_2D const & c, point_2D const & d) { // line2 (c,d)
    const point_2D a_b = b - a;
    const point_2D c_d = d - c;
    return (fsign(rotate(a_b, c - b)) * fsign(rotate(a_b, d - b)) <= 0) &&
           (fsign(rotate(c_d, a - d)) * fsign(rotate(c_d, b - d)) <= 0);
}
#endif

bool math_util::line_rect_intersect(point_2D const & a, point_2D const & b, rect_2D const & rc) {
    point_2D const & lb = rc.lb();
    if (line_intersect(a, b, rc.lt, lb)) return true;
    if (line_intersect(a, b, lb, rc.rb)) return true;
    point_2D const & rt = rc.rt();
    if (line_intersect(a, b, rc.rb, rt)) return true;
    if (line_intersect(a, b, rt, rc.lt)) return true;
    return false;
}

bool math_util::polyline_rect_intersect(point_2D const * const begin, point_2D const * const end, rect_2D const & rc)
{
    SDL_ASSERT(begin <= end);
    if (!(begin < end)) {
        return false;
    }
    SDL_ASSERT(rc.lt < rc.rb);
    auto first = end - 1;
    for (auto second = begin; second != end; ++second) {
        point_2D const & p1 = *first;   // vert[j]
        point_2D const & p2 = *second;  // vert[i]
        if (line_rect_intersect(p1, p2, rc)) {
            return true;
        }
        first = second;
    }
    if (point_inside(begin[0], rc)) { // test any point of contour
        return true; // contour inside rect
    }
    return false;
}

math_util::contains_t
math_util::contains(point_2D const * const begin, point_2D const * const end, rect_2D const & rc)
{
    SDL_ASSERT(begin <= end);
    if (!(begin < end)) {
        return contains_t::none;
    }
    SDL_ASSERT(rc.lt < rc.rb);
    auto first = end - 1;
    bool crossing = false;
    for (auto second = begin; second != end; ++second) {
        point_2D const & p1 = *first;   // vert[j]
        point_2D const & p2 = *second;  // vert[i]
        if (line_rect_intersect(p1, p2, rc)) {
            return contains_t::intersect;
        }
        if (ray_crossing(rc.lt, p1, p2)) {
            crossing = !crossing; // At each crossing, the ray switches between inside and outside
        }
        first = second;
    }
    // no intersection between contour and rect found
    if (crossing) {
        return contains_t::rect_inside;
    }
    if (point_inside(begin[0], rc)) { // test any point of contour
        return contains_t::poly_inside;
    }
    return contains_t::none;
}

bool math_util::polygon_intersect(spatial_point const * const begin,
                                   spatial_point const * const end, 
                                   spatial_rect const & rc)
{
    if (!rc) {
        SDL_ASSERT(0);
        return false;
    }
    static_assert(sizeof(point_2D) == sizeof(spatial_point), "");
    static_assert(sizeof(rect_2D) == sizeof(spatial_rect), "");
    return contains(
        reinterpret_cast<point_2D const *>(begin),
        reinterpret_cast<point_2D const *>(end),
        reinterpret_cast<rect_2D const &>(rc)) != contains_t::none;
}

bool math_util::linestring_intersect(spatial_point const * const begin, 
                                      spatial_point const * const end,
                                      spatial_rect const & rc)
{
    if (!rc) {
        SDL_ASSERT(0);
        return false;
    }
    static_assert(sizeof(point_2D) == sizeof(spatial_point), "");
    static_assert(sizeof(rect_2D) == sizeof(spatial_rect), "");
    return polyline_rect_intersect(
        reinterpret_cast<point_2D const *>(begin),
        reinterpret_cast<point_2D const *>(end),
        reinterpret_cast<rect_2D const &>(rc));
}

bool math_util::point_in_polygon(spatial_point const * const first,
                                 spatial_point const * const last,
                                 spatial_point const & test)
{
    SDL_ASSERT(first <= last);
    if (!(first < last)) {
        return false;
    }
    bool interior = false; // true : point is inside polygon
    auto p1 = first;
    auto p2 = p1 + 1;
    if (*p1 == test) 
        return true;
    while (p2 < last) {
        SDL_ASSERT(p1 < p2);
        if (*p2 == test)
            return true;
        auto const & v1 = *(p2 - 1);
        auto const & v2 = *p2;
        if (((v1.latitude > test.latitude) != (v2.latitude > test.latitude)) &&
            ((test.longitude + limits::fepsilon) < ((test.latitude - v2.latitude) * 
                (v1.longitude - v2.longitude) / (v1.latitude - v2.latitude) + v2.longitude))) {
            interior = !interior;
        }
        if (*p1 == *p2) { // end of ring found
            ++p2;
            p1 = p2;
        }
        ++p2;
    }
    return interior;
}

bool math_util::point_in_polygon(spatial_point const * const first,
                                 spatial_point const * const last,
                                 spatial_point const & test,
                                 bool & point_on_vertix)
{
    SDL_ASSERT(first <= last);
    point_on_vertix = false;
    if (!(first < last)) {
        return false;
    }
    bool interior = false; // true : point is inside polygon
    auto p1 = first;
    auto p2 = p1 + 1;
    if (*p1 == test) 
        return point_on_vertix = true;
    while (p2 < last) {
        SDL_ASSERT(p1 < p2);
        if (*p2 == test)
            return point_on_vertix = true;
        auto const & v1 = *(p2 - 1);
        auto const & v2 = *p2;
        if (((v1.latitude > test.latitude) != (v2.latitude > test.latitude)) &&
            ((test.longitude + limits::fepsilon) < ((test.latitude - v2.latitude) * 
                (v1.longitude - v2.longitude) / (v1.latitude - v2.latitude) + v2.longitude))) {
            interior = !interior;
        }
        if (*p1 == *p2) { // end of ring found
            ++p2;
            p1 = p2;
        }
        ++p2;
    }
    return interior;
}

winding math_util::ring_winding(spatial_point const * first, spatial_point const * last)
{
    //https://en.wikipedia.org/wiki/Shoelace_formula
    //http://mathworld.wolfram.com/PolygonArea.html

    SDL_ASSERT(first + 1 < last);
    SDL_ASSERT(*first == *(last - 1));

    if (first < --last) {
        SDL_ASSERT(last - first); // number of sides of the polygon
        double x1, y0, sum = 0;
        spatial_point const * p0 = last;
        while (first != last) {
            y0 = p0->latitude; p0 = first;
            x1 = first->longitude; ++first;
            sum += x1 * (first->latitude - y0); // y2 = first->latitude
        }
        SDL_ASSERT(!fzero(sum));
        // Note that the area of a convex polygon is defined to be positive
        // if the points are arranged in a counterclockwise order, 
        // and negative if they are in clockwise order (Beyer 1987).
        return (sum < 0) ?
            winding::clockwise :        // interior
            winding::counterclockwise;  // exterior
    }
    return winding::exterior;
}

} // db
} // sdl

#if SDL_DEBUG
namespace sdl {
    namespace db {
        namespace {
            class unit_test {
            public:
                unit_test()
                {
                    {
                        point_2D const a = { -1, 0 };
                        point_2D const b = { 1, 0 };
                        point_2D const c = { 0, -1 };
                        point_2D const d = { 0, 1 };
                        point_2D p = {-1, -1};
                        SDL_ASSERT(math_util::get_line_intersect(p, a, b, c, d));
                        SDL_ASSERT(fequal(p.X, 0));
                        SDL_ASSERT(fequal(p.Y, 0));
                    }
                    {
                        spatial_point const exterior[] = { // POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))
                            { 10, 30 }, // Y = latitude, X = longitude
                            { 40, 40 },
                            { 40, 20 },
                            { 20, 10 },
                            { 10, 30 },
                        };
                        SDL_ASSERT(is_counterclockwise(math_util::ring_winding(std::begin(exterior), std::end(exterior))));
                        spatial_point const interior[] = { //(20 30, 35 35, 30 20, 20 30)
                            { 30, 20 }, // Y = latitude, X = longitude
                            { 35, 35 },
                            { 20, 30 },
                            { 30, 20 },
                        };
                        SDL_ASSERT(is_clockwise(math_util::ring_winding(std::begin(interior), std::end(interior))));
                    }
                }
            };
            static unit_test s_test;
        }
    } // db
} // sdl
#endif //#if SDL_DEBUG


#if 0 //reserved
template<class vector_type>
vector_type sort_unique(vector_type && result) {
    if (!result.empty()) {
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
    }
    return std::move(result); // must return copy
}

template<class vector_type>
vector_type sort_unique(vector_type && v1, vector_type && v2) {
    if (v1.empty()) {
        return sort_unique(std::move(v2));
    }
    if (v2.empty()) {
        return sort_unique(std::move(v1));
    }
    v1.insert(v1.end(), v2.begin(), v2.end());
    return sort_unique(std::move(v1));
}
#endif