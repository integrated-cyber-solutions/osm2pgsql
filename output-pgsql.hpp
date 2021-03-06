/* Implements the output-layer processing for osm2pgsql
 * storing the data in several PostgreSQL tables
 * with the final PostGIS geometries for each entity
*/

#ifndef OUTPUT_PGSQL_H
#define OUTPUT_PGSQL_H

#include "output.hpp"
#include "tagtransform.hpp"
#include "geometry-builder.hpp"
#include "reprojection.hpp"
#include "expire-tiles.hpp"
#include "id-tracker.hpp"
#include "table.hpp"

#include <vector>
#include <boost/shared_ptr.hpp>

class output_pgsql_t : public output_t {
public:
    enum table_id {
        t_point = 0, t_line, t_poly, t_roads, t_MAX
    };

    output_pgsql_t(const middle_query_t* mid_, const options_t &options_);
    virtual ~output_pgsql_t();
    output_pgsql_t(const output_pgsql_t& other);

    virtual boost::shared_ptr<output_t> clone(const middle_query_t* cloned_middle) const;

    int start();
    void stop();
    void commit();

    void enqueue_ways(pending_queue_t &job_queue, osmid_t id, size_t output_id, size_t& added);
    int pending_way(osmid_t id, int exists);

    void enqueue_relations(pending_queue_t &job_queue, osmid_t id, size_t output_id, size_t& added);
    int pending_relation(osmid_t id, int exists);

    int node_add(osmid_t id, double lat, double lon, struct keyval *tags);
    int way_add(osmid_t id, osmid_t *nodes, int node_count, struct keyval *tags);
    int relation_add(osmid_t id, struct member *members, int member_count, struct keyval *tags);

    int node_modify(osmid_t id, double lat, double lon, struct keyval *tags);
    int way_modify(osmid_t id, osmid_t *nodes, int node_count, struct keyval *tags);
    int relation_modify(osmid_t id, struct member *members, int member_count, struct keyval *tags);

    int node_delete(osmid_t id);
    int way_delete(osmid_t id);
    int relation_delete(osmid_t id);

    size_t pending_count() const;

    void merge_pending_relations(boost::shared_ptr<output_t> other);
    void merge_expire_trees(boost::shared_ptr<output_t> other);
    virtual boost::shared_ptr<id_tracker> get_pending_relations();
    virtual boost::shared_ptr<expire_tiles> get_expire_tree();

protected:

    int pgsql_out_node(osmid_t id, struct keyval *tags, double node_lat, double node_lon);
    int pgsql_out_way(osmid_t id, struct keyval *tags, const struct osmNode *nodes, int count, int exists);
    int pgsql_out_relation(osmid_t id, struct keyval *rel_tags, int member_count, const struct osmNode * const * xnodes, struct keyval *xtags, const int *xcount, const osmid_t *xid, const char * const *xrole, bool pending);
    int pgsql_process_relation(osmid_t id, const struct member *members, int member_count, struct keyval *tags, int exists, bool pending=false);
    int pgsql_delete_way_from_output(osmid_t osm_id);
    int pgsql_delete_relation_from_output(osmid_t osm_id);

    boost::scoped_ptr<tagtransform> m_tagtransform;

    //enable output of a generated way_area tag to either hstore or its own column
    int m_enable_way_area;

    std::vector<boost::shared_ptr<table_t> > m_tables;

    boost::scoped_ptr<export_list> m_export_list;

    geometry_builder builder;

    boost::shared_ptr<reprojection> reproj;
    boost::shared_ptr<expire_tiles> expire;

    boost::shared_ptr<id_tracker> ways_pending_tracker, ways_done_tracker, rels_pending_tracker;

    const static std::string NAME;
};

#endif
