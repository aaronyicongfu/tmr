#ifdef TMR_HAS_OPENCASCADE

#include "TMROpenCascade.h"
#include "TMRMesh.h"
#include "TACSMeshLoader.h"
#include "MITCShell.h"
#include "isoFSDTStiffness.h"

int main( int argc, char *argv[] ){
  MPI_Init(&argc, &argv);
  TMRInitialize();

  // Get the communicator
  MPI_Comm comm = MPI_COMM_WORLD;

  char filename[256];
  sprintf(filename, "misc1.step");

  // Flag to indicate whether to write/test a BDF file
  int test_bdf_file = 0;

  double htarget = 4.0;
  for ( int k = 0; k < argc; k++ ){
    if (sscanf(argv[k], "h=%lf", &htarget) == 1){
      if (htarget < 0.1){ htarget = 0.1; }
    }
    if (sscanf(argv[k], "file=%s", filename) == 1){
      printf("file=%s\n", filename);
    }
    if (strcmp(argv[k], "--test_bdf") == 0){
      test_bdf_file = 1;
    }
  }

  // Load in the geometry file
  TMRModel *geo = TMR_LoadModelFromSTEPFile(filename);
  if (geo){
    geo->incref();

    // Get the vertices
    int num_verts;
    TMRVertex **verts;
    geo->getVertices(&num_verts, &verts);

    // Get the edges
    int num_edges;
    TMREdge **edges;
    geo->getEdges(&num_edges, &edges);

    // Separate and plot the different surfaces
    int num_faces;
    TMRFace **faces;
    geo->getFaces(&num_faces, &faces);

    TMRModel *model = new TMRModel(num_verts, verts,
                                   num_edges, edges, 
                                   num_faces, faces);

    // Allocate the new mesh
    TMRMesh *mesh = new TMRMesh(MPI_COMM_WORLD, model);
    mesh->incref();

    // Adjust the quality factor
    TMRMeshOptions options;
    options.frontal_quality_factor = 1.5;
    options.num_smoothing_steps = 10;

    // Mesh the object of interest
    mesh->mesh(options, htarget);
    mesh->writeToVTK("surface-mesh.vtk");

    if (test_bdf_file){
      mesh->writeToBDF("surface-mesh.bdf");
    }

    // Decref the objects
    mesh->decref();
    geo->decref();
  }

  if (test_bdf_file){
    TACSMeshLoader *loader = new TACSMeshLoader(comm);
    loader->incref();

    loader->scanBDFFile("surface-mesh.bdf");

    // Create the solid stiffness object
    isoFSDTStiffness *stiff = 
      new isoFSDTStiffness(1.0, 1.0, 0.3, 0.833, 1.0, 1.0);
    MITCShell<2> *elem = new MITCShell<2>(stiff);

    for ( int i = 0; i < loader->getNumComponents(); i++ ){ 
      loader->setElement(i, elem);
    }
    
    // Create the TACSAssembler object
    TACSAssembler *tacs = loader->createTACS(6);
    tacs->incref();

    // Create the f5 visualization object
    TACSToFH5 *f5 = loader->createTACSToFH5(tacs, TACS_SHELL, 
                                            TACSElement::OUTPUT_NODES);
    f5->incref();
    f5->writeToFile("surface-mesh.f5");
  }
  
  TMRFinalize();
  MPI_Finalize();

  return 0;
}

#endif // TMR_HAS_OPENCASCADE