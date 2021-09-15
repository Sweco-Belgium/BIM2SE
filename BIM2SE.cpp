#include <iostream>
#include <BIM2SE.h>

// OCCT library
// - Basic data structures (geometry)
// - Modeling algorithms
// - Working with mesh (faceted) data
// - Data interoperability

// Grouped in packages > toolkits (libraries; .so or .dll) > modules

// Data types:
// 1. Primitive types (Boolean, Character, Integer, Real,..) --> Manipulated by value
// 2. OCCT classes --> Manupulated by handle (= on reference to an instance)
// Handle = safe way to manipulate object

#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx> // Primitive type of the Standard package
#include <Standard_Transient.hxx> // Primitive type of the Standard package

// Retrieve type descriptor -> STANDARD_TYPE(Geom_Line) for example
#include <gp_Pnt.hxx> // Point - short lived
#include <NCollection_Array2.hxx> // To define 2D Arrays
#include <TColgp_Array2OfPnt.hxx> 

#include <Geom_Point.hxx> // Type manipulated by handle - Abstract class
#include <Geom_CartesianPoint.hxx>  // Concrete class
#include <Geom_BSplineSurface.hxx> 

// GeomAPI 
#include <GeomAPI_PointsToBSplineSurface.hxx> // Surface approximation

// OCCT BrepPrimAPI
#include <BRepPrimAPI_MakeBox.hxx> // Make a Box
#include <BRepPrimAPI_MakeCylinder.hxx> // Make a Cylinder
#include <BRepMesh_IncrementalMesh.hxx> // Make a mesh from a topological data structure
#include <BRepBuilderAPI_MakeFace.hxx> // Build a face from a surface

// OCCT Boolean operation / Algoritms
#include <BRepAlgoAPI_Cut.hxx>

// Writa a STL file
#include <StlAPI_Writer.hxx>

// Write a STEPfile
#include <STEPControl_Writer.hxx>

// Include GProp package - calculate global properties
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

// Type definitions
typedef NCollection_Array2<gp_Pnt> ListOfPoints; // the same as TColgp_Array2OfPnt

// Include goodbye if set
#ifdef USE_SANDBOX
#   include <sandbox.h>
#endif

int main(int argc, char *argv[])
{
  // Introduction
  std::cout << "BIM2SE - (c) " << AUTHOR << std::endl;

  /*
    In[1]: Define a BOX geometry AND cut away a volume
  */

  // Create a simple box
  gp_Pnt lowerLeftCornerOfBox (-50.0, -50.0, 0.0);
  BRepPrimAPI_MakeBox boxMaker (lowerLeftCornerOfBox, 100, 100, 100);
  TopoDS_Shape box = boxMaker.Shape();

  // Create a cylinder
  BRepPrimAPI_MakeCylinder cylinderMaker(25.0, 50.0);
  TopoDS_Shape cylinder = cylinderMaker.Shape();

  // Cut the cylinder out of the box
  BRepAlgoAPI_Cut cutMaker(box, cylinder);
  TopoDS_Shape boxWithHole = cutMaker.Shape();

  // Make the export writers ready
  STEPControl_Writer STEPwriter;
  StlAPI_Writer STLwriter;

  /*
    In[2]: Define a surface using 4 points - approximate 
  */

  // Define 4 point which will be used to approximate a surface
  gp_Pnt aPnt1 (79, 87, 26);
  gp_Pnt aPnt2 (-62, 93, 84);
  gp_Pnt aPnt3 (-97, -61, 3);
  gp_Pnt aPnt4 (65, -65, 65);

  // Construct a surface - Generate 2Array of Points
  // Piont(i, j) where i is used for U, j for V
  TColgp_Array2OfPnt aPointList (1, 2, 1, 2);
  aPointList.SetValue(1, 1, aPnt1);
  aPointList.SetValue(1, 2, aPnt2);
  aPointList.SetValue(2, 2, aPnt3);
  aPointList.SetValue(2, 1, aPnt4);

  // Approximate the surface - interpolation would also be an option (same API)
  GeomAPI_PointsToBSplineSurface aPointsToBSplineSurface;
  #ifdef USE_SANDBOX
    aPointsToBSplineSurface = (aPointList);  
  #endif

  // Check if the surface has been created
  if(aPointsToBSplineSurface.IsDone())
  {
    // If the surface has been created, print a message
    std::cout << "Creation of Surface succeeded!" << std::endl;
    // Return the surface - downcasting
    Handle(Geom_BSplineSurface) aBSplineSurface = aPointsToBSplineSurface.Surface();
    // Handle(Geom_Surface) aSurface = Handle(Geom_Surface)::DownCast(aBSplineSurface);
    // Convert the surface to a face - second param the tolerance
    TopoDS_Face soilSurface = BRepBuilderAPI_MakeFace(aBSplineSurface, 1e-6);
    
    // Write the face (soilSurface) to a file
    STEPwriter.Transfer(soilSurface, STEPControl_AsIs);
    STEPwriter.Write("soilSurface.stp");
    // Write to an .stl file - First create a meshed surface
    BRepMesh_IncrementalMesh meshedSoilSurface (soilSurface, 1e-2, Standard_True);
    TopoDS_Shape soilSurfaceForExport = meshedSoilSurface.Shape();
    STLwriter.Write(soilSurfaceForExport, "soilSurface.stl");
  } else {
    // If no surface is created, we write the original geometry
    STEPwriter.Transfer(boxWithHole, STEPControl_AsIs);
    STEPwriter.Write("originalGeometry.stp"); 
    // To write an STL file, we first need to mesh our geometry 
    BRepMesh_IncrementalMesh meshedGeometry (boxWithHole, 1e-2, Standard_True);
    TopoDS_Shape geometryForExport = meshedGeometry.Shape();
    STLwriter.Write(geometryForExport, "originalGeometry.stl");
    // Calculate the volume of the cut geometry
    GProp_GProps volumeProperties;
    BRepGProp::VolumeProperties(boxWithHole, volumeProperties);
    std::cout << std::setprecision(5) << "Volume of the model is: " << volumeProperties.Mass() << std::endl;
    // Calculated the volume of the entire cube
    BRepGProp::VolumeProperties(box, volumeProperties);
    std::cout << std::setprecision(5) << "Volume of the original model is: " << volumeProperties.Mass() << std::endl;
  }

  return 0;
}
