/* This file is part of MODEL, the Mechanics Of Defect Evolution Library.
 *
 * Copyright (C) 2011 by Giacomo Po <gpo@ucla.edu>.
 *
 * model is distributed without any warranty under the
 * GNU General Public License (GPL) v2 <http://www.gnu.org/licenses/>.
 */

#ifndef model_DDinteractionStyle_H_
#define model_DDinteractionStyle_H_

#include <memory>
#include <stdlib.h>     //for using the function sleep
#include <fstream>

#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
//#include <vtkInteractorStyleMultiTouchCamera.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkBMPWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkPropPicker.h>
#include <vtkRendererCollection.h>
#include <vtkObjectFactory.h>

#include <DislocationSegmentActor.h>
//#include <PKActor.h>
//#include <GlidePlaneActor.h>
#include <InclusionActor.h>
#include <TerminalColors.h>
//#include <EigenDataReader.h>
#include <TextFileParser.h>
#include <PlotActor.h>
#include <DDauxVtk.h>
#include <DDconfigVtk.h>
#include <FieldActor.h>
#include <BVPsolverBase.h>

namespace model
{
    
    //To update the display once you get new data, you would just update the
    //PolyData that is already attached to a mapper->actor->renderer (and
    //                                                                call Modified() on it if necessary) and the renderer would
    //automatically display the new points.
    
    class DDinteractionStyle : public vtkInteractorStyleTrackballCamera
    //    public vtkInteractorStyleMultiTouchCamera
    {
        
    public:
        vtkSmartPointer<vtkRenderer> ddRenderer;

    private:
//        std::unique_ptr<PKActor> ddPK;
//        std::unique_ptr<GlidePlaneActor> ddAux;
        std::unique_ptr<DDauxVtk> ddAux;

        std::unique_ptr<InclusionActor> inclusions;
        std::unique_ptr<PlotActor> plot;
        vtkSmartPointer<vtkAxesActor> axes;
        vtkSmartPointer<vtkOrientationMarkerWidget> widget;

        
        int xCol;
        int yCol;
        double winFrac;
        std::string selectedKey;
        bool saveImage;
        int imageType;
        int imageMagnification;
        bool imageTransparentBackground;
        long int frameIncrement;
        long int currentFrameID;
        long int lastFrameID;
        vtkActor    *LastPickedActor;
        vtkProperty *LastPickedProperty;
        
        std::map<int,std::string> FlabelsMap;
        bool autoSpin;
        double degPerStep;
        Eigen::Matrix<double,3,1> spinAxis;
        
        bool axisWidgetEnabled;
    public:
        
        static DDinteractionStyle* New();
        vtkTypeMacro(DDinteractionStyle, vtkInteractorStyleTrackballCamera);
        
        SimplicialMesh<3> mesh;
        Polycrystal<3> poly;
        BVPsolverBase<3> bvpSolver;
        SimplicialMeshActor meshActor;
        DDconfigVtk ddConfig;
        FieldActor fieldActor;

//        std::unique_ptr<DislocationSegmentActor> ddSegments;
//        DislocationSegmentActor ddSegments;
//        vtkRenderer* ddRenderer;
        vtkRenderer* plotRenderer;
        
        /**********************************************************************/
        DDinteractionStyle() :
        /* init */ ddRenderer(vtkSmartPointer<vtkRenderer>::New())
        /* init */,axes(vtkSmartPointer<vtkAxesActor>::New())
        /* init */,widget(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
        /* init */,xCol(0)
        /* init */,yCol(2)
        /* init */,winFrac(0.5)
        /* init */,saveImage(false)
        /* init */,imageType(1)
        /* init */,imageMagnification(1)
        /* init */,imageTransparentBackground(false)
        /* init */,frameIncrement(TextFileParser("./inputFiles/DD.txt").readScalar<int>("outputFrequency",false))
        /* init */,currentFrameID(-1)
        /* init */,lastFrameID(currentFrameID)
        /* init */,autoSpin(false)
        /* init */,degPerStep(0.0)
        /* init */,spinAxis(Eigen::Matrix<double,3,1>::Zero())
        /* init */,axisWidgetEnabled(true)
//        /* init */,mesh(TextFileParser("./inputFiles/polycrystal.txt").readString("meshFile",true))
        /* init */,mesh(TextFileParser("./inputFiles/polycrystal.txt").readString("meshFile",true),TextFileParser("./inputFiles/polycrystal.txt").readMatrix<double>("A",3,3,true),TextFileParser("./inputFiles/polycrystal.txt").readMatrix<double>("x0",1,3,true).transpose())
        /* init */,poly("./inputFiles/polycrystal.txt",mesh)
        /* init */,bvpSolver(poly)
        /* init */,meshActor(ddRenderer,mesh)
        /* init */,ddConfig(ddRenderer,mesh)
        /* init */,fieldActor(meshActor,ddConfig,bvpSolver,meshActor.clipPlane,ddRenderer)
//        /* init */,ddSegments(0/*,lastFrameID,degPerStep,spinAxis*/,ddRenderer,meshActor.mesh)
        {
            
//            meshActor.ddSegments=&ddSegments.ddSegments;
            
            ddRenderer->SetBackground(1,1,1); // Background color white
            ddRenderer->SetViewport(0.0,0,0.5,1);
//            ddRenderer->ResetCamera();
            this->SetDefaultRenderer(ddRenderer);



            LastPickedActor = NULL;
            LastPickedProperty = vtkProperty::New();
            
//            model::EigenDataReader EDR;
//            EDR.readScalarInFile("./DDinput.txt","outputFrequency",frameIncrement);
            
            //                        PlotActor pa(plotRenderer);
            
            std::string filename("F/F_labels.txt");
            std::ifstream ifs ( filename.c_str() , std::ifstream::in );
            if (ifs.is_open())
            {
                std::string line;
                int colID=0;
                while (std::getline(ifs, line))
                {
                    FlabelsMap.emplace(colID,line);
                    colID++;
                }
            }
            else
            {
                std::cout<<"CANNOT READ "<<filename<<std::endl;
            }
            
            
        }
        
        
        void init(vtkRenderer* _plotRenderer)
        {
        
//            ddRenderer=_ddRenderer;
            plotRenderer=_plotRenderer;
            
//            meshActor.init(ddRenderer);
            
//            ddSegments.reset(new DislocationSegmentActor(frameID/*,lastFrameID,degPerStep,spinAxis*/,ddRenderer,meshActor.mesh));
            
            loadFrame(0);
            
            plotRenderer->SetBackground(1,1,1);
            plotRenderer->SetViewport(0.5,0,1.0,1);
            
            widget->SetOutlineColor( 0.9300, 0.5700, 0.1300 );
            widget->SetOrientationMarker( axes );
            widget->SetInteractor( this->Interactor );
            widget->SetViewport( 0.0, 0.0, 0.4, 0.4 );
            widget->SetEnabled( axisWidgetEnabled );
            widget->InteractiveOn();

            fieldActor.planeWidget->SetInteractor(this->Interactor);
//            fieldActor.planeWidget->On();
            
            plotRenderer->SetBackground(1,1,1);
            plotRenderer->SetViewport(0.5,0,1.0,1);
            
//            ddRenderer->ResetCamera();
//            renderWindow->Render();
        }
        
        
        /**********************************************************************/
        bool loadFrame(const long int& frameID)
        {
            
            bool frameLoaded=false;
            DDconfigIO<3> temp;
            if(   currentFrameID!=frameID
               && (temp.isBinGood(frameID) || temp.isTxtGood(frameID))
               )
            {
                lastFrameID=currentFrameID;
                currentFrameID=frameID;
                std::cout<<greenBoldColor<<"Loading frame "<<currentFrameID<<defaultColor<<std::endl;
                
                vtkRenderWindowInteractor *rwi = this->Interactor;
                
                if (this->LastPickedActor)
                {// Before destroying all actors,restore properties of LastPickedActor
                    this->LastPickedActor->GetProperty()->DeepCopy(this->LastPickedProperty);
                    LastPickedActor = NULL; // LastPickedActor will be destroyed so we cannot further pick it
                }
                
                // Update ddActors
                ddConfig.updateConfiguration(frameID);
                
                ddAux.reset(new DDauxVtk(frameID,ddRenderer));
                if(bvpSolver.displacement().gSize()==ddAux->displacementFEM.size())
                {
                   bvpSolver.displacement()=ddAux->displacementFEM;
                }
                else
                {
                    bvpSolver.displacement()=Eigen::VectorXd::Zero(bvpSolver.displacement().gSize());
                }
                if(bvpSolver.vacancyConcentration().gSize()==ddAux->vacancyFEM.size())
                {
                    bvpSolver.vacancyConcentration()=ddAux->vacancyFEM;
                }
                else
                {
                    bvpSolver.vacancyConcentration()=Eigen::VectorXd::Zero(bvpSolver.vacancyConcentration().gSize());
                }
                
                
                
                meshActor.update(*ddAux/*,lastFrameID,degPerStep,spinAxis*/);
//                ddSegments.updateConfiguration(frameID);
//                ddSegments.reset(new DislocationSegmentActor(frameID/*,lastFrameID,degPerStep,spinAxis*/,ddRenderer,meshActor.mesh));
//                ddPK.reset(new PKActor(frameID,ddRenderer));
                inclusions.reset(new InclusionActor(0,ddRenderer));
                plot.reset(new PlotActor(plotRenderer,xCol,yCol,currentFrameID,FlabelsMap));
                if(fieldActor.meshActor->GetVisibility())
                {
                    fieldActor.compute();
                }
                    

                const double spinAxisNorm(spinAxis.norm());
                if(autoSpin && degPerStep && spinAxisNorm)
                {
                    spinAxis/=spinAxisNorm;
                    
                    Eigen::AngleAxisd R(degPerStep*M_PI/180.0,spinAxis);
                    double viewUp[3];
                    this->GetDefaultRenderer( )->GetActiveCamera( )->GetViewUp( viewUp[0], viewUp[1], viewUp[2] );
                    
                    double focalPoint[3];
                    this->GetDefaultRenderer( )->GetActiveCamera( )->GetFocalPoint( focalPoint[0], focalPoint[1], focalPoint[2] );
                    
                    double cameraPosition[3];
                    this->GetDefaultRenderer( )->GetActiveCamera( )->GetPosition( cameraPosition[0], cameraPosition[1], cameraPosition[2] );

                    Eigen::Map<Eigen::Vector3d> ViewUp(viewUp);
                    Eigen::Map<Eigen::Vector3d> FocalPoint(focalPoint);
                    Eigen::Map<Eigen::Vector3d> CameraPosition(cameraPosition);
                    
                    const Eigen::Vector3d newPosition=FocalPoint+R*(CameraPosition-FocalPoint);
                    const Eigen::Vector3d newViewUp=R*ViewUp;
                    
                    this->GetDefaultRenderer()->GetActiveCamera()->SetViewUp( newViewUp(0), newViewUp(1), newViewUp(2) );
//                    this->GetDefaultRenderer()->GetActiveCamera()->SetFocalPoint( focalPoint[0], focalPoint[1], focalPoint[2] );
                    this->GetDefaultRenderer()->GetActiveCamera()->SetPosition( newPosition[0], newPosition[1], newPosition[2] );

                }
                
                // Update renderer
                rwi->Render();
                
                if (saveImage)
                {
                    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
                    windowToImageFilter->SetInput(rwi->GetRenderWindow()/*renderWindow*/);
//                    windowToImageFilter->SetMagnification(imageMagnification); //set the resolution of the output image (3 times the current resolution of vtk render window)
                    if(imageTransparentBackground)
                    {
                        windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
                    }
//                    windowToImageFilter->ReadFrontBufferOff(); // read from the back buffer
                    windowToImageFilter->Update();
                    
                    //                    int imageType=0;
                    switch (imageType)
                    {
                        case 1:
                        {
                            const std::string fileName="png/image_"+std::to_string(frameID)+".png";
                            std::cout<<"saving image "<<fileName<<std::endl;
                            vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
                            writer->SetFileName(fileName.c_str());
                            writer->SetInputConnection(windowToImageFilter->GetOutputPort());
                            writer->Write();
                            break;
                            
                        }
                            
                        case 2:
                        {
                            const std::string fileName="jpg/image_"+std::to_string(frameID)+".jpg";
                            std::cout<<"saving image "<<fileName<<std::endl;
                            vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
                            writer->SetFileName(fileName.c_str());
                            writer->SetInputConnection(windowToImageFilter->GetOutputPort());
                            writer->Write();
                            break;
                            
                        }
                            
                        case 3:
                        {
                            const std::string fileName="bmp/image_"+std::to_string(frameID)+".bmp";
                            std::cout<<"saving image "<<fileName<<std::endl;
                            vtkSmartPointer<vtkBMPWriter> writer = vtkSmartPointer<vtkBMPWriter>::New();
                            writer->SetFileName(fileName.c_str());
                            writer->SetInputConnection(windowToImageFilter->GetOutputPort());
                            writer->Write();
                            break;
                            
                        }
                            
                            
                        default:
                            break;
                    }
                    
                    //                    rwi->ResetCamera();
                    rwi->Render();
                }
                frameLoaded=true;
            }
            else
            {
                std::cout<<"frame "<<frameID<<" not found. Reverting to "<<currentFrameID<<std::endl;
                //                frameID=currentFrameID; // frameID is not a valid ID, return to last read
            }
            return frameLoaded;
        }
        
        
        
        

        
        
        /**********************************************************************/
        virtual ~DDinteractionStyle()
        {
            LastPickedProperty->Delete();
        }
        
        /**********************************************************************/
        virtual void OnRightButtonDown() override
        {
            
            double viewUp[3];
            this->GetDefaultRenderer( )->GetActiveCamera( )->GetViewUp( viewUp[0], viewUp[1], viewUp[2] );
            
            double focalPoint[3];
            this->GetDefaultRenderer( )->GetActiveCamera( )->GetFocalPoint( focalPoint[0], focalPoint[1], focalPoint[2] );
            
            double cameraPosition[3];
            this->GetDefaultRenderer( )->GetActiveCamera( )->GetPosition( cameraPosition[0], cameraPosition[1], cameraPosition[2] );
            
            double viewAngle= this->GetDefaultRenderer( )->GetActiveCamera( )->GetViewAngle(  );
            
            double parallelScale=this->GetDefaultRenderer( )->GetActiveCamera( )->GetParallelScale( );
            
            
            std::cout<<greenBoldColor<<"writing to cameraState.txt"<<defaultColor<<std::endl;
            std::cout<<"viewUp="<<viewUp[0]<<" "<<viewUp[1]<<" "<<viewUp[2]<<std::endl;
            std::cout<<"focalPoint="<<focalPoint[0]<<" "<<focalPoint[1]<<" "<<focalPoint[2]<<std::endl;
            std::cout<<"cameraPosition="<<cameraPosition[0]<<" "<<cameraPosition[1]<<" "<<cameraPosition[2]<<std::endl;
            std::cout<<"viewAngle="<<viewAngle<<std::endl;
            std::cout<<"parallelScale="<<parallelScale<<std::endl;
            
            std::ofstream myfile;
            myfile.open ("cameraState.txt");
            myfile<<viewUp[0]<<" "<<viewUp[1]<<" "<<viewUp[2]<<std::endl;
            myfile<<focalPoint[0]<<" "<<focalPoint[1]<<" "<<focalPoint[2]<<std::endl;
            myfile<<cameraPosition[0]<<" "<<cameraPosition[1]<<" "<<cameraPosition[2]<<std::endl;
            myfile<<viewAngle<<std::endl;
            myfile<<parallelScale<<std::endl;
            myfile.close();
            
            // Forward events
            vtkInteractorStyleTrackballCamera::OnRightButtonDown();
        }
        
        /*************************************************************************/
        virtual void OnLeftButtonDown() override
        {
//            autoSpin=false;
            
            int* clickPos = this->GetInteractor()->GetEventPosition();
            
            // Pick from this location.
            vtkSmartPointer<vtkPropPicker>  picker =
            vtkSmartPointer<vtkPropPicker>::New();
            picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
            
            // If we picked something before, reset its property
            if (this->LastPickedActor)
            {
                this->LastPickedActor->GetProperty()->DeepCopy(this->LastPickedProperty);
            }
            this->LastPickedActor = picker->GetActor();
            if (this->LastPickedActor)
            {
                // Save the property of the picked actor so that we can
                // restore it next time
                this->LastPickedProperty->DeepCopy(this->LastPickedActor->GetProperty());
                // Highlight the picked actor by changing its properties
                this->LastPickedActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                this->LastPickedActor->GetProperty()->SetDiffuse(1.0);
                this->LastPickedActor->GetProperty()->SetSpecular(0.0);
            }
            
            // Forward events
            vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
        }
        
        /*************************************************************************/
        virtual void OnChar() override
        {/*! Overrides vtkInteractorStyleTrackballCamera::OnChar()
          * to avoid exiting the program on pressing "e"
          */
        }
        
        /*************************************************************************/
        virtual void OnKeyPress() override
        {
            // Get the keypress
            vtkRenderWindowInteractor *rwi = this->Interactor;
            std::string key = rwi->GetKeySym();
            
            if(key == "Escape")
            {
                std::cout << "Exiting DDvtk, goodbye!" << std::endl;
                exit(0);
            }
            // Handle an arrow key
            else if(key == "Up")
            {
                //                frameID-=frameIncrement;
                const bool success=loadFrame(currentFrameID-frameIncrement);
                if(this->Interactor->GetShiftKey() && success)
                {
                    //                    std::cout << "Shift held. ";
                    std::cout<<"Loading all frames"<<std::endl;
                    OnKeyPress();
                }
                
            }
            else if(key == "Down")
            {
                //                frameID+=frameIncrement;
                const bool success=loadFrame(currentFrameID+frameIncrement);
                
                if(this->Interactor->GetShiftKey() && success)
                {
                    //                    std::cout << "Shift held. ";
                    std::cout<<"Loading all frames"<<std::endl;
                    OnKeyPress();
                }
            }
            else if(key == "Right")
            {
                winFrac+=0.1;
                if(winFrac>1.0)
                {
                    winFrac=1.0;
                }
                ddRenderer ->SetViewport(0.0,0,winFrac,1);
                plotRenderer->SetViewport(winFrac,0,1,1);
                
                ddRenderer->Render();
                plotRenderer->Render();
                this->Interactor->Render();
            }
            
            if(key == "Left")
            {                winFrac-=0.1;
                if(winFrac<0.0)
                {
                    winFrac=0.0;
                }
                
                ddRenderer->SetViewport(0.0,0,winFrac,1);
                plotRenderer->SetViewport(winFrac,0,1,1);
                
                ddRenderer->Render();
                plotRenderer->Render();
                this->Interactor->Render();
            }
            else if(key == "a")
            {
                selectedKey="a";
                std::cout<<"selecting objects: axis"<<std::endl;
                axisWidgetEnabled=!axisWidgetEnabled;
                widget->SetEnabled( axisWidgetEnabled );
                ddRenderer->Render();
                this->Interactor->Render();
            }
            else if(key == "c")
            {
                autoSpin=false;
                
                std::cout<<greenBoldColor<<"reading cameraState.txt"<<defaultColor<<std::endl;
                std::ifstream ifs ( "cameraState.txt" , std::ifstream::in );
                if (ifs.is_open())
                {
                    std::string line;
                    
                    std::getline(ifs, line);
                    double viewUp[3];
                    std::stringstream viewUpss(line);
                    viewUpss>>viewUp[0]>>viewUp[1]>>viewUp[2];
                    
                    std::getline(ifs, line);
                    double focalPoint[3];
                    std::stringstream focalPointss(line);
                    focalPointss>>focalPoint[0]>>focalPoint[1]>>focalPoint[2];
                    
                    std::getline(ifs, line);
                    double cameraPosition[3];
                    std::stringstream cameraPositionss(line);
                    cameraPositionss>>cameraPosition[0]>>cameraPosition[1]>>cameraPosition[2];
                    
                    std::getline(ifs, line);
                    double viewAngle;
                    std::stringstream viewAngless(line);
                    viewAngless>>viewAngle;
                    
                    std::getline(ifs, line);
                    double parallelScale;
                    std::stringstream parallelScaless(line);
                    parallelScaless>>parallelScale;
                    
                    
                    std::cout<<"viewUp="<<viewUp[0]<<" "<<viewUp[1]<<" "<<viewUp[2]<<std::endl;
                    std::cout<<"focalPoint="<<focalPoint[0]<<" "<<focalPoint[1]<<" "<<focalPoint[2]<<std::endl;
                    std::cout<<"cameraPosition="<<cameraPosition[0]<<" "<<cameraPosition[1]<<" "<<cameraPosition[2]<<std::endl;
                    std::cout<<"viewAngle="<<viewAngle<<std::endl;
                    std::cout<<"parallelScale="<<parallelScale<<std::endl;
                    
                    this->GetDefaultRenderer()->GetActiveCamera()->SetViewUp( viewUp[0], viewUp[1], viewUp[2] );
                    this->GetDefaultRenderer()->GetActiveCamera()->SetFocalPoint( focalPoint[0], focalPoint[1], focalPoint[2] );
                    this->GetDefaultRenderer()->GetActiveCamera()->SetPosition( cameraPosition[0], cameraPosition[1], cameraPosition[2] );
                    this->GetDefaultRenderer( )->GetActiveCamera( )->SetViewAngle( viewAngle );
                    this->GetDefaultRenderer( )->GetActiveCamera( )->SetParallelScale( parallelScale );
                    
                    this->Interactor->Render();
                    
                }
                else
                {
                    std::cout<<"file not found."<<std::endl;
                }
                ifs.close();
                
                
                
            }
            else if(key == "e")
            {
                selectedKey="e";
                std::cout<<"selecting objects: dislocation segments"<<std::endl;
                std::cout<<"    +/- to increase tube radius"<<std::endl;
                std::cout<<"      0 to show/hide zero-Burgers vector segments"<<std::endl;
                std::cout<<"      1 to show/hide boundary segments"<<std::endl;
                std::cout<<"      2 to turn on/off segment radii scaled by Burgers norm"<<std::endl;
                std::cout<<"      3 to color grain-boundary segments in black "<<std::endl;
                std::cout<<"      4 to color segments by Burgers vector "<<std::endl;
                std::cout<<"      5 to color segments by glissile/sessile "<<std::endl;
                std::cout<<"      6 to color segments by screw/edge "<<std::endl;
                std::cout<<"      7 to color segments by planeNormal "<<std::endl;
                std::cout<<"      8 show/hide glide planes "<<std::endl;
                
            }
            else if(key == "f")
            {
                selectedKey="f";
//                fieldActor.enableWidget=true;
//                fieldActor.planeWidget->SetEnabled( fieldActor.enableWidget );
//                fieldActor.meshActor->VisibilityOff();
                ddRenderer->Render();
                this->Interactor->Render();
                std::cout<<"selecting objects: field plane"<<std::endl;
//                std::cout<<"    +/- to increase tube radius"<<std::endl;
                std::cout<<"      0 to show/hide plane widget"<<std::endl;
                std::cout<<"      1 to remesh plane"<<std::endl;
                std::cout<<"      2 compute fields"<<std::endl;
                std::cout<<"      3 select field"<<std::endl;
                std::cout<<"      4 select colorbar limits"<<std::endl;
                std::cout<<"      5 enter mesh size in [b^2]"<<std::endl;
                std::cout<<"      6 toggle colorbar visibility"<<std::endl;


            }
            else if(key == "g")
            {
                if(selectedKey[0]=='g')
                {
                    selectedKey=" ";
                    if(ddAux!=nullptr)
                    {
                        ddAux->showGlidePlanes=false;
//                        ddAux->showPeriodicGlidePlanes=false;
//                        ddAux->showPeriodicLoops=false;
                        ddAux->modify();
                        this->Interactor->Render();
                        std::cout<<"showGlidePlanes="<<ddAux->showGlidePlanes<<std::endl;
//                        std::cout<<"showPeriodicGlidePlanes="<<ddAux->showPeriodicGlidePlanes<<std::endl;
                    }
                }
                else
                {
                    selectedKey="g";
                    std::cout<<"selecting objects: glide planes"<<std::endl;
//                    std::cout<<"    1 to show periodic glide planes"<<std::endl;
//                    std::cout<<"    2 to show periodic loops"<<std::endl;
                    std::cout<<"    +/- to increase opacity"<<std::endl;
                    if(ddAux!=nullptr)
                    {
                        ddAux->showGlidePlanes=true;
                        ddAux->modify();
                        this->Interactor->Render();
                        std::cout<<"showGlidePlanes="<<ddAux->showGlidePlanes<<std::endl;
                    }
                }
            }
            else if(key == "i")
            {
                std::cout<<"Enter frame increment (>0): "<<std::flush;
                long int temp;
                std::cin>>temp;
                if(temp>0)
                {
                    frameIncrement=temp;
                }
                else
                {
                    std::cout<<"frame increment must be >0. Reverting to frame increment="<<frameIncrement<<std::endl;
                }
            }
            else if(key == "l")
            {// std::cin return the current element of the input buffer, if the buffer is empty it waits for user input
                autoSpin=false;
                std::cout<<"Enter frame# to load:"<<std::endl;
                long int frameID;
                std::cin>>frameID;
                std::cout<<"entered "<<frameID<<std::endl;
                loadFrame(frameID);
            }
            else if(key == "m")
            {
                selectedKey="m";
                std::cout<<"selecting objects: mesh"<<std::endl;
                std::cout<<"    +/- to increase mesh displacement"<<std::endl;
                std::cout<<"    1 to show/hide grain colors"<<std::endl;
                std::cout<<"    2 to show/hide region boundaries"<<std::endl;
                std::cout<<"    3 to show/hide face boundaries"<<std::endl;
                std::cout<<"    4 to show/hide mesh"<<std::endl;

            }
            else if(key == "q")
            {
                if(selectedKey[0]=='q')
                {
                    selectedKey=" ";
                    if(ddAux.get()!=nullptr)
                    {
                        DDauxVtk::showPkforces=false;
                        DDauxVtk::showSfforces=false;
                        DDauxVtk::showGlideVelocities=false;
                        ddAux->modify();
                        this->Interactor->Render();
                    }
                }
                else
                {
                    selectedKey="q";
                    std::cout<<"selecting objects: quadrature points"<<std::endl;
                    std::cout<<"    0 show/hide PK-forces"<<std::endl;
                    std::cout<<"    1 show/hide glide velocities"<<std::endl;
//                    std::cout<<"    2 show/hide stacking-fault forces"<<std::endl;

//                    if(ddPK.get()!=nullptr)
//                    {
//                        PKActor::showPK=true;
//                        ddPK->modify();
//                        this->Interactor->Render();
//                    }
                    
                }
            }
            else if(key == "0" && selectedKey[0]=='q')
            {
                if(selectedKey=="q0")
                {
//                    selectedKey=" ";
                    DDauxVtk::showPkforces=!DDauxVtk::showPkforces;
                    DDauxVtk::showSfforces=!DDauxVtk::showSfforces;

                }
                else
                {
                    selectedKey="q0";
                    DDauxVtk::showPkforces=true;
                    DDauxVtk::showSfforces=true;

                }
//                selectedKey="q0";
//                DDauxVtk::showPkforces=!DDauxVtk::showPkforces;
                std::cout<<"quadrature PK forces="<<DDauxVtk::showPkforces<<std::endl;
                std::cout<<"quadrature SF forces="<<DDauxVtk::showSfforces<<std::endl;

                ddAux->modify();
                this->Interactor->Render();
                if(DDauxVtk::showPkforces)
                {
                    std::cout<<"    +/- to increase PK forces size"<<std::endl;
                }
            }
            else if(key == "1" && selectedKey[0]=='q')
            {
                if(selectedKey=="q1")
                {
//                    selectedKey=" ";
                    DDauxVtk::showGlideVelocities=!DDauxVtk::showGlideVelocities;
                }
                else
                {
                    selectedKey="q1";
                    DDauxVtk::showGlideVelocities=true;
                }
//                selectedKey="q1";
//                DDauxVtk::showGlideVelocities=!DDauxVtk::showGlideVelocities;
                std::cout<<"quadrature glide velocities="<<DDauxVtk::showGlideVelocities<<std::endl;
                ddAux->modify();
                this->Interactor->Render();
                if(DDauxVtk::showGlideVelocities)
                {
                    std::cout<<"    +/- to increase glide velocities size"<<std::endl;
                }
            }
            else if(key == "r")
            {
                autoSpin=true;
                selectedKey="a";
                std::cout<<"Enter spin axis x y z: "<<std::flush;
                float x,y,z;
                std::cin>>x>>y>>z;
                spinAxis<<x,y,z;
                std::cout<<"Enter spin angle per step (deg/step): "<<std::flush;
                std::cin>>degPerStep;
            }
            else if(key == "s")
            {
                selectedKey="s";
                //saveImage=true;
                std::cout<<"Save images menu:"<<std::endl;
                std::cout<<"    press space to start/stop saving images"<<std::endl;
                std::cout<<"    press 1 to save in png format"<<std::endl;
                std::cout<<"    press 2 to save in jgp format"<<std::endl;
                std::cout<<"    press 3 to save in bmp format"<<std::endl;
                std::cout<<"    press 0 to enable/disable transparent background"<<std::endl;
                std::cout<<"    press +/- to increase/decrease image resolution"<<std::endl;
                
            }
            else if(key == "x")
            {
                std::cout<<"Enter column of x-axis data:"<<std::endl;
                for(const auto& pair : FlabelsMap)
                {
                    std::cout<<pair.first<<"    "<<pair.second<<"\n";
                }
                std::cin>>xCol;
                if(xCol<0)
                {
                    std::cout<<"wrong column number "<<xCol<<std::flush;
                    xCol=0;
                    std::cout<<". reverting to "<<xCol<<std::endl;
                }
                plot.reset(new PlotActor(plotRenderer,xCol,yCol,currentFrameID,FlabelsMap));
                this->Interactor->Render();
            }
            else if(key == "y")
            {
                std::cout<<"Enter column of y-axis data:"<<std::endl;
                for(const auto& pair : FlabelsMap)
                {
                    std::cout<<pair.first<<"    "<<pair.second<<"\n";
                }
                std::cin>>yCol;
                if(yCol<0)
                {
                    std::cout<<"wrong column number "<<yCol<<std::flush;
                    yCol=1;
                    std::cout<<". reverting to "<<yCol<<std::endl;
                }
                plot.reset(new PlotActor(plotRenderer,xCol,yCol,currentFrameID,FlabelsMap));
                this->Interactor->Render();
            }
            else if(key == "t")
            {
                if(selectedKey[0]=='t')
                {
                    selectedKey=" ";
                        DDconfigVtkBase::showSlippedArea=false;
                        ddConfig.modify();
                        this->Interactor->Render();
                }
                else
                {
                    selectedKey="t";
                    std::cout<<"selecting objects: Slipped Areas"<<std::endl;
                    std::cout<<"    +/- to increase/decrease opacity"<<std::endl;
                    
                        DDconfigVtkBase::showSlippedArea=true;
                        ddConfig.modify();
                        this->Interactor->Render();
                }
            }
            else if(key == "v")
            {
                if(selectedKey[0]=='v')
                {
                    selectedKey=" ";
                        DDconfigVtkBase::showNodes=false;
                        ddConfig.modify();
                        this->Interactor->Render();
                }
                else
                {
                    selectedKey="v";
                    std::cout<<"selecting objects: Dislocation Nodes"<<std::endl;
                    std::cout<<"      1 to show/hide node IDs"<<std::endl;
                    std::cout<<"      2 to show/hide a specific node ID"<<std::endl;
                    
                        DDconfigVtkBase::showNodes=true;
                        ddConfig.modify();
                        this->Interactor->Render();
                }
            }
            else if(key == "w")
            {
                if(selectedKey[0]=='w')
                {
                    selectedKey=" ";
                        DDconfigVtkBase::showVelocities=false;
                        ddConfig.modify();
                        this->Interactor->Render();
                }
                else
                {
                    selectedKey="w";
                    std::cout<<"selecting objects: nodal velocities"<<std::endl;
                    std::cout<<"    +/- to increase vector size"<<std::endl;
                        DDconfigVtkBase::showVelocities=true;
                        ddConfig.modify();
                        this->Interactor->Render();
                }
            }
            
            if(selectedKey[0]=='e')
            {
                
                if(key == "equal")
                {
                    DDconfigVtkBase::tubeRadius*=2.0;
                    ddConfig.modify();
                    std::cout<<"tube radius="<<DDconfigVtkBase::tubeRadius<<std::endl;
                    this->Interactor->Render();
                }
                if(key == "minus")
                {
                    DDconfigVtkBase::tubeRadius*=0.5;
                    ddConfig.modify();
                    std::cout<<"tube radius="<<DDconfigVtkBase::tubeRadius<<std::endl;
                    this->Interactor->Render();
                }
                if(key == "0")
                {
                    DDconfigVtkBase::showZeroBuergers=!DDconfigVtkBase::showZeroBuergers;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                if(key == "1")
                {
                    DDconfigVtkBase::showBoundarySegments=!DDconfigVtkBase::showBoundarySegments;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                if(key == "2")
                {
                    DDconfigVtkBase::scaleRadiusByBurgers=!DDconfigVtkBase::scaleRadiusByBurgers;
                    std::cout<<"scaleRadiusByBurgers="<<DDconfigVtkBase::scaleRadiusByBurgers<<std::endl;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                if(key == "3")
                {
                    DDconfigVtkBase::blackGrainBoundarySegments=!DDconfigVtkBase::blackGrainBoundarySegments;
                    std::cout<<"blackGrainBoundarySegments="<<DDconfigVtkBase::blackGrainBoundarySegments<<std::endl;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                if(key == "4")
                {
                    DDconfigVtkBase::clr=DDconfigVtkBase::colorBurgers;
                    std::cout<<"DislocationSegment color scheme = Burgers. Reload frame to update colors."<<std::endl;
//                    ddConfig.modify();
//                    this->Interactor->Render();
                }
                if(key == "5")
                {
                    DDconfigVtkBase::clr=DDconfigVtkBase::colorSessile;
                    std::cout<<"DislocationSegment color scheme = Glissile/Sessile. Reload frame to update colors."<<std::endl;
                    //                    ddConfig.modify();
                    //                    this->Interactor->Render();
                }
                if(key == "6")
                {
                    DDconfigVtkBase::clr=DDconfigVtkBase::colorEdgeScrew;
                    std::cout<<"DislocationSegment color scheme = screw/edge. Reload frame to update colors."<<std::endl;
                    //                    ddConfig.modify();
                    //                    this->Interactor->Render();
                }
                if(key == "7")
                {
                    DDconfigVtkBase::clr=DDconfigVtkBase::colorNormal;
                    std::cout<<"DislocationSegment color scheme = planeNormal. Reload frame to update colors."<<std::endl;
                    //                    ddConfig.modify();
                    //                    this->Interactor->Render();
                }
//                if(key == "8")
//                {
//                    DDconfigVtkBase::showGlidePlanes=!DDconfigVtkBase::showGlidePlanes;
//                    std::cout<<"show GlidePlanes="<<DDconfigVtkBase::showGlidePlanes<<std::endl;
//                    ddConfig.modify();
//                    //                    ddConfig.modify();
//                    //                    this->Interactor->Render();
//                }
                
            }
            else if(selectedKey[0]=='f')
            {
                if(key == "0")
                {
                    //                fieldActor.planeWidget->SetEnabled( fieldActor.enableWidget );
                    //                fieldActor.meshActor->VisibilityOff();
                    fieldActor.enableWidget=!fieldActor.enableWidget;
                    fieldActor.planeWidget->SetEnabled( fieldActor.enableWidget );
                    fieldActor.meshActor->VisibilityOff();
                    ddRenderer->Render();
                    this->Interactor->Render();
                }
                else if(key == "1")
                {
                    fieldActor.enableWidget=false;
                    fieldActor.planeWidget->SetEnabled( fieldActor.enableWidget );
                    fieldActor.meshActor->VisibilityOn();
                    fieldActor.remesh();
                    ddRenderer->Render();
                    this->Interactor->Render();
                }
                else if(key == "2")
                {
                    fieldActor.enableWidget=false;
                    fieldActor.planeWidget->SetEnabled( fieldActor.enableWidget );
                    fieldActor.meshActor->VisibilityOn();
                    fieldActor.compute();
                    ddRenderer->Render();
                    this->Interactor->Render();
                }
                else if(key == "3")
                {
                    fieldActor.enableWidget=false;
                    fieldActor.planeWidget->SetEnabled( fieldActor.enableWidget );
                    fieldActor.meshActor->VisibilityOn();
                    std::cout<<"Available fields:"<<std::endl;
                    std::cout<<"0: sigma_11 (DD)"<<std::endl;
                    std::cout<<"1: sigma_12 (DD)"<<std::endl;
                    std::cout<<"2: sigma_13 (DD)"<<std::endl;
                    std::cout<<"3: sigma_22 (DD)"<<std::endl;
                    std::cout<<"4: sigma_23 (DD)"<<std::endl;
                    std::cout<<"5: sigma_33 (DD)"<<std::endl;
                    
                    std::cout<<"6: sigma_11 (FEM)"<<std::endl;
                    std::cout<<"7: sigma_12 (FEM)"<<std::endl;
                    std::cout<<"8: sigma_13 (FEM)"<<std::endl;
                    std::cout<<"9: sigma_22 (FEM)"<<std::endl;
                    std::cout<<"10: sigma_23 (FEM)"<<std::endl;
                    std::cout<<"11: sigma_33 (FEM)"<<std::endl;

                    std::cout<<"12: sigma_11 (DD+FEM)"<<std::endl;
                    std::cout<<"13: sigma_12 (DD+FEM)"<<std::endl;
                    std::cout<<"14: sigma_13 (DD+FEM)"<<std::endl;
                    std::cout<<"15: sigma_22 (DD+FEM)"<<std::endl;
                    std::cout<<"16: sigma_23 (DD+FEM)"<<std::endl;
                    std::cout<<"17: sigma_33 (DD+FEM)"<<std::endl;

                    std::cout<<"18: vacancy concentration (DD)"<<std::endl;
                    std::cout<<"19: vacancy concentration (FEM)"<<std::endl;
                    std::cout<<"20: vacancy concentration (DD+FEM)"<<std::endl;
                    std::cout<<"Enter field ID to plot:"<<std::flush;
                    std::cin >> FieldActor::plotChoice;
                    std::cout <<std::endl;
                    fieldActor.plotField();
                    ddRenderer->Render();
                    this->Interactor->Render();
                }
                else if(key == "4")
                {
                    std::cout<<"Enter colorbar minimum limits:"<<std::flush;
                    std::cin >> FieldActor::lutMin;
                    std::cout<<"Enter colorbar maximum limits:"<<std::flush;
                    std::cin >> FieldActor::lutMax;
                    fieldActor.plotField();
                    ddRenderer->Render();
                    this->Interactor->Render();
                }
                else if(key == "5")
                {
                    fieldActor.meshActor->VisibilityOn();
                    std::cout<<"Enter mesh size in [b^2]:"<<std::flush;
                    std::cin >> FieldActor::meshSize;
                    std::cout <<std::endl;
                    if(FieldActor::meshSize<=0.0)
                    {
                        FieldActor::meshSize=100;
                        std::cout<<"invalid mesh size, reverting to meshSize="<<FieldActor::meshSize<<std::endl;
                    }
                    else
                    {
                        fieldActor.remesh();
                    }
                }
                else if(key == "6")
                {
                    fieldActor.scalarBar->SetVisibility(!fieldActor.scalarBar->GetVisibility());
                    std::cout<<"scalarBar visibility="<<fieldActor.scalarBar->GetVisibility()<<std::flush;
                    this->Interactor->Render();
                }
            }
            else if(selectedKey[0]=='m')
            {
                if(key == "equal")
                {
                    SimplicialMeshActor::dispCorr*=2.0;
                    std::cout<<"displacement amplification="<<SimplicialMeshActor::dispCorr<<std::endl;
                    meshActor.modifyPts();
                    this->Interactor->Render();
                }
                if(key == "minus")
                {
                    SimplicialMeshActor::dispCorr*=0.5;
                    std::cout<<"displacement amplification="<<SimplicialMeshActor::dispCorr<<std::endl;
                    meshActor.modifyPts();
                    this->Interactor->Render();
                }
                if(key == "1")
                {
                    SimplicialMeshActor::showGrainColors=!SimplicialMeshActor::showGrainColors;
                    std::cout<<"showGrainColors="<<SimplicialMeshActor::showGrainColors<<std::endl;
                    meshActor.modifyPts();
                    this->Interactor->Render();
                }
                if(key == "2")
                {
                    SimplicialMeshActor::showRegionBoundaries=!SimplicialMeshActor::showRegionBoundaries;
                    std::cout<<"showRegionBoundaries="<<SimplicialMeshActor::showRegionBoundaries<<std::endl;
                    meshActor.modifyPts();
                    this->Interactor->Render();
                }
                if(key == "3")
                {
                    SimplicialMeshActor::showFaceBoundaries=!SimplicialMeshActor::showFaceBoundaries;
                    std::cout<<"showFaceBoundaries="<<SimplicialMeshActor::showFaceBoundaries<<std::endl;
                    meshActor.modifyPts();
                    this->Interactor->Render();
                }
                if(key == "4")
                {
                    SimplicialMeshActor::showMesh=!SimplicialMeshActor::showMesh;
                    std::cout<<"showMesh="<<SimplicialMeshActor::showMesh<<std::endl;
                    meshActor.modifyPts();
                    this->Interactor->Render();
                }
                
            }
            else if(selectedKey=="q0")
            {
                if(key == "equal" && ddAux.get()!=nullptr)
                {
                    ddAux->pkFactor*=2.0;
                    ddAux->sfFactor*=2.0;
                    ddAux->modify();
                    std::cout<<"PK force scaling="<<ddAux->pkFactor<<std::endl;
                    std::cout<<"SF force scaling="<<ddAux->sfFactor<<std::endl;
                    this->Interactor->Render();
                }
                if(key == "minus" && ddAux.get()!=nullptr)
                {
                    ddAux->pkFactor*=0.5;
                    ddAux->sfFactor*=0.5;
                    ddAux->modify();
                    std::cout<<"PK force scaling="<<ddAux->pkFactor<<std::endl;
                    std::cout<<"SF force scaling="<<ddAux->sfFactor<<std::endl;
                    this->Interactor->Render();
                }
            }
            else if(selectedKey=="q1")
            {
                if(key == "equal" && ddAux.get()!=nullptr)
                {
                    ddAux->glideVelocitiesFactor*=2.0;
                    ddAux->modify();
                    std::cout<<"glide velocity scaling="<<ddAux->glideVelocitiesFactor<<std::endl;
                    this->Interactor->Render();
                }
                if(key == "minus" && ddAux.get()!=nullptr)
                {
                    ddAux->glideVelocitiesFactor*=0.5;
                    ddAux->modify();
                    std::cout<<"glide velocity scaling="<<ddAux->glideVelocitiesFactor<<std::endl;
                    this->Interactor->Render();
                }
            }
            else if(selectedKey[0]=='s')
            {
                if(key == "space")
                {
                    saveImage=!saveImage;
                    if(saveImage)
                    {
                        std::cout<<"saving images ON"<<std::endl;
                    }
                    else
                    {
                        std::cout<<"saving images OFF"<<std::endl;
                    }
                }
                
                if(key == "1" && saveImage)
                {
                    imageType=1;
                    std::cout<<"selecting png output"<<std::endl;
                }
                
                if(key == "2" && saveImage)
                {
                    imageType=2;
                    std::cout<<"selecting jpg output"<<std::endl;
                }
                
                if(key == "3" && saveImage)
                {
                    imageType=3;
                    std::cout<<"selecting bmp output"<<std::endl;
                }
                
                if(key == "0" && saveImage)
                {
                    imageTransparentBackground=!imageTransparentBackground;
                    if(imageTransparentBackground)
                    {
                        std::cout<<"transparent background ON"<<std::endl;
                    }
                    else
                    {
                        std::cout<<"transparent background OFF"<<std::endl;
                    }
                }
                
                if(key == "equal" && saveImage)
                {
                    imageMagnification++;
                    std::cout<<"image magnification "<<imageMagnification<<"X"<<std::endl;
                    
                }
                
                if(key == "minus" && saveImage)
                {
                    imageMagnification--;
                    if(imageMagnification<1)
                    {
                        imageMagnification=1;
                    }
                    std::cout<<"image magnification "<<imageMagnification<<"X"<<std::endl;
                }
                
            }
            else if(selectedKey[0]=='v')
            {
                if(key == "1")
                {
                    DDconfigVtkBase::showNodeIDs=!DDconfigVtkBase::showNodeIDs;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                
                if(key == "2")
                {
                    DDconfigVtkBase::showSingleNode=!DDconfigVtkBase::showSingleNode;
                    if(DDconfigVtkBase::showSingleNode)
                    {
                        std::cout << "Enter node ID "<<std::flush;
                        std::cin >> DDconfigVtkBase::singleNodeID;
                        std::cout <<std::endl;
                    }
                    ddConfig.modify();
                    this->Interactor->Render();
                }
            }
            else if(selectedKey[0]=='w')
            {
                if(key == "equal" )
                {
                    DDconfigVtkBase::velocityFactor*=2.0;
                    ddConfig.modify();
                    std::cout<<"velocity scaling="<<DDconfigVtkBase::velocityFactor<<std::endl;
                    this->Interactor->Render();
                }
                if(key == "minus" )
                {
                    DDconfigVtkBase::velocityFactor*=0.5;
                    ddConfig.modify();
                    std::cout<<"velocity scaling="<<DDconfigVtkBase::velocityFactor<<std::endl;
                    this->Interactor->Render();
                }
                
            }
            else if(selectedKey[0]=='g')
            {
                if(key == "equal" && ddAux!=nullptr)
                {
                    ddAux->glidePlaneOpacity*=1.25;
                    ddAux->modify();
                    this->Interactor->Render();
                    std::cout<<"glidePlaneOpacity="<<ddAux->glidePlaneOpacity<<std::endl;
                }
                if(key == "minus" && ddAux!=nullptr)
                {
                    ddAux->glidePlaneOpacity*=0.8;
                    ddAux->modify();
                    this->Interactor->Render();
                    std::cout<<"glidePlaneOpacity="<<ddAux->glidePlaneOpacity<<std::endl;
                }
//                if(key == "1" && ddAux!=nullptr)
//                {
//                    ddAux->showPeriodicGlidePlanes=!ddAux->showPeriodicGlidePlanes;
//                    ddAux->modify();
//                    this->Interactor->Render();
//                    std::cout<<"showPeriodicGlidePlanes="<<ddAux->showPeriodicGlidePlanes<<std::endl;
//                }
//                if(key == "2" && ddAux!=nullptr)
//                {
//                    ddAux->showPeriodicLoops=!ddAux->showPeriodicLoops;
//                    ddAux->modify();
//                    this->Interactor->Render();
//                    std::cout<<"showPeriodicLoops="<<ddAux->showPeriodicLoops<<std::endl;
//                }
            }
            else if(selectedKey[0]=='t')
            {
                if(key == "equal" )
                {
                    DDconfigVtkBase::slippedAreaOpacity*=1.25;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                if(key == "minus" )
                {
                    DDconfigVtkBase::slippedAreaOpacity/=1.25;
                    ddConfig.modify();
                    this->Interactor->Render();
                }
                
            }
            
            
            
            // Forward events
            vtkInteractorStyleTrackballCamera::OnKeyPress();// use base class OnKeyPress()
            /*
             http://www.vtk.org/doc/nightly/html/classvtkInteractorStyle.htmlf
             Keypress j / Keypress t: toggle between joystick (position sensitive) and trackball (motion sensitive) styles. In joystick style, motion occurs continuously as long as a mouse button is pressed. In trackball style, motion occurs when the mouse button is pressed and the mouse pointer moves.
             Keypress c / Keypress a: toggle between camera and actor modes. In camera mode, mouse events affect the camera position and focal point. In actor mode, mouse events affect the actor that is under the mouse pointer.
             Button 1: rotate the camera around its focal point (if camera mode) or rotate the actor around its origin (if actor mode). The rotation is in the direction defined from the center of the renderer's viewport towards the mouse position. In joystick mode, the magnitude of the rotation is determined by the distance the mouse is from the center of the render window.
             Button 2: pan the camera (if camera mode) or translate the actor (if actor mode). In joystick mode, the direction of pan or translation is from the center of the viewport towards the mouse position. In trackball mode, the direction of motion is the direction the mouse moves. (Note: with 2-button mice, pan is defined as <Shift>-Button 1.)
             Button 3: zoom the camera (if camera mode) or scale the actor (if actor mode). Zoom in/increase scale if the mouse position is in the top half of the viewport; zoom out/decrease scale if the mouse position is in the bottom half. In joystick mode, the amount of zoom is controlled by the distance of the mouse pointer from the horizontal centerline of the window.
             Keypress 3: toggle the render window into and out of stereo mode. By default, red-blue stereo pairs are created. Some systems support Crystal Eyes LCD stereo glasses; you have to invoke SetStereoTypeToCrystalEyes() on the rendering window.
             Keypress e: exit the application.
             Keypress f: fly to the picked point
             Keypress p: perform a pick operation. The render window interactor has an internal instance of vtkCellPicker that it uses to pick.
             Keypress r: reset the camera view along the current view direction. Centers the actors and moves the camera so that all actors are visible.
             Keypress s: modify the representation of all actors so that they are surfaces.
             Keypress u: invoke the user-defined function. Typically, this keypress will bring up an interactor that you can type commands in. Typing u calls UserCallBack() on the vtkRenderWindowInteractor, which invokes a vtkCommand::UserEvent. In other words, to define a user-defined callback, just add an observer to the vtkCommand::UserEvent on the vtkRenderWindowInteractor object.
             Keypress w: modify the representation of all actors so that they are wireframe.
             */
        }
        
    };
    
    vtkStandardNewMacro(DDinteractionStyle);
    
    
} // namespace model
#endif







