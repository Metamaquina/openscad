
#ifdef ENABLE_CGAL

#include "printutils.h"
#include "CGAL_Nef_polyhedron.h"


// todo -- cleanup comments

// todo -- add OFF export option to use tesselation or non-tesselation

// todo -- actually test code

// Convert Nef Polyhedron3 to Polyhedron3 without tesssellating the faces
// into triangles. For example a ten-sided cylinder should have a top face
// with nine vertices and ten edges, not be split into ten triangles.
// CGAL's default Nef3->convert_to_polyhedron() function does tessellation.
// Sometimes we need to avoid tesssellation, for example for
// Surface Subdivision.

// this class helps build a Polyhedron from a Nef Polyhedron
// See the Polyhedron Builder cgal manual, as well as // cgalutils.cc
// CGAL_Build_Polyset.
/*
begin_surface
  (add_vertex | (begin_facet add_vertex_to_facet* end_facet))*
end_surface
*/
class Builder : public CGAL::Modifier_base<CGAL_HDS>
{
public:
 	const CGAL_Nef_polyhedron3 &nef;
	Builder(const CGAL_Nef_polyhedron3 &nef) : nef(nef) { }
	void operator()(CGAL_HDS& hds)
	{
		CGAL_Polybuilder B(hds, true);
		std::vector<CGAL_HDS::Vertex::Point> vertices;
		CGAL_Nef_polyhedron3::Vertex_const_iterator vi;
		std::map<CGAL_HDS::Vertex::Point,int> vertmap1;
		std::map<int,CGAL_HDS::Vertex::Point> vertmap2;
		int vertcount = 0;

		B.begin_surface(nef.number_of_vertices(), nef.number_of_facets());
		CGAL_forall_vertices( vi, nef ) {
			B.add_vertex( vi->point() );
			vertmap1[ vi->point() ] = vertcount;
			vertmap2[ vertcount ] = vi->point();
			vertcount++;
		}
		assert( vertcount == nef.number_of_vertices() );

		CGAL_Nef_polyhedron3::Halffacet_const_iterator hfaceti;
		CGAL_forall_halffacets( hfaceti, nef ) {
			if (hfaceti->incident_volume()->mark() == 0) continue;
			if (hfaceti->mark() == 0) continue;
			B.begin_facet();
			CGAL_Nef_polyhedron3::Halffacet_cycle_const_iterator cyclei;
			int cycle_count = 0;
	    CGAL_forall_facet_cycles_of( cyclei, hfaceti ) {
				if ( cycle_count == 0 ) {
					PRINT("body contour. adding");
				}
				else {
					PRINT("hole? skipping (not implemented)"); continue;
				}
				CGAL_Nef_polyhedron3::SHalfedge_around_facet_const_circulator c1(cyclei);
				CGAL_Nef_polyhedron3::SHalfedge_around_facet_const_circulator c2(c1);
				CGAL_For_all( c1, c2 ) {
	        CGAL_Point_3 source = c1->source()->source()->point();
					B.add_vertex_to_facet( vertmap1[ source ] );
				}
				cycle_count++;
			}
			B.end_facet();
		}
		B.end_surface();
	}
};



void CGAL_Nef_polyhedron::convertToPolyhedron( CGAL_Polyhedron &P, bool tessellate ) const
{
	assert(dim==3);
  CGAL::Failure_behaviour old_behaviour = CGAL::set_error_behaviour(CGAL::THROW_EXCEPTION);
  try {
		if (tessellate) {
  	 	this->p3->convert_to_Polyhedron(P);
		}
		else {
			P.clear();
			Builder builder( *this->p3 );
			P.delegate(builder);
		}
  }
  catch (const CGAL::Assertion_exception &e) {
    PRINTB("CGAL error in nef->polyhedron conversion: %s", e.what());
  }
  CGAL::set_error_behaviour(old_behaviour);
}


#endif /* ENABLE_CGAL */


