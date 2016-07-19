#ifndef TMR_OCTANT_FOREST_H
#define TMR_OCTANT_FOREST_H

#include "TMROctree.h"

class TMROctForest {
 public:
  TMROctForest( MPI_Comm _comm );
  ~TMROctForest();

  // Set the connectivity
  // --------------------
  void setConnectivity( int _num_nodes,
                        const int *_block_conn,
                        int _num_blocks );

  // Create the forest of octrees
  // ----------------------------
  void createTrees( int refine_level );
  void createRandomTrees( int nrand=10, 
                          int min_level=0, int max_level=8 );

  // Duplicate or coarsen the forest
  // -------------------------------
  TMROctForest* duplicate();
  TMROctForest *coarsen();

  // Balance the octree meshes
  // -------------------------
  void balance( int balance_corner=0 );

  // Create the nodes
  // ----------------
  void createNodes( int order=2 );

  // Get the array of Octrees
  // ------------------------
  int getOctrees( TMROctree ***_trees ){
    if (_trees){ *_trees = octrees; }
    return num_blocks;
  }

 private:
  // Balance-related routines
  // ------------------------
  // Balance the octant across the local tree and the forest
  void balanceOctant( int block, TMROctant *oct,
                      TMROctantHash **hash, TMROctantQueue **queue,
                      const int balance_corner,
                      const int balance_tree );

  // Add adjacent quadrants to the hashes/queues for balancing
  void addFaceNeighbors( int block, int face_index, 
                         TMROctant p,
                         TMROctantHash **hash,
                         TMROctantQueue **queue );
  void addEdgeNeighbors( int block, int edge_index, 
                         TMROctant p,
                         TMROctantHash **hash,
                         TMROctantQueue **queue );
  void addCornerNeighbors( int block, int corner, 
                           TMROctant p,
                           TMROctantHash **hash,
                           TMROctantQueue **queue );

  // Nodal ordering routines
  // -----------------------
  // Add octants to adjacent non-owner processor queues
  void addEdgeOctantToQueues( const int edge, 
                              const int mpi_rank, 
                              TMROctant *q,
                              TMROctantQueue **queues );
  void addFaceOctantToQueues( const int face, 
                              const int mpi_rank, 
                              TMROctant *q,
                              TMROctantQueue **queues );

  // Exchange non-local octant neighbors
  void exchangeOctNeighbors( const int *face_block_owners,
                             const int *edge_block_owners );
    
  // The communicator
  MPI_Comm comm;

  // Set the nodes/edges/faces/blocks
  int num_nodes, num_edges, num_faces, num_blocks;

  // Information for the face/edge/node connectivity
  int *block_conn, *block_face_conn, *block_edge_conn;
  int *node_block_ptr, *node_block_conn;
  int *edge_block_ptr, *edge_block_conn;
  int *face_block_ptr, *face_block_conn;

  // Information to enable transformations between faces
  int *block_face_ids;

  // Information about the mesh
  int mesh_order;

  // Set the elements
  int num_elements;

  // Set the range of nodes owned by each processor
  int *node_range;

  // Keep a pointer to the forest of quadtrees
  TMROctree **octrees; 

  // The mpi rank of the face owner
  int *mpi_block_owners;
};

#endif // TMR_OCTANT_FOREST_H