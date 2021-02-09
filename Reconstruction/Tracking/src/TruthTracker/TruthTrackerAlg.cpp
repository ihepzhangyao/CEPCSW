#include "TruthTrackerAlg.h"
#include "DataHelper/HelixClass.h"
#include "DDSegmentation/Segmentation.h"
#include "DetSegmentation/GridDriftChamber.h"
#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DD4hep/IDDescriptor.h"
#include "DD4hep/Plugins.h"
#include "DD4hep/DD4hepUnits.h"
#include "UTIL/ILDConf.h"

//external
#include "CLHEP/Random/RandGauss.h"
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoParticleAssociationCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"

DECLARE_COMPONENT(TruthTrackerAlg)

TruthTrackerAlg::TruthTrackerAlg(const std::string& name, ISvcLocator* svcLoc)
: GaudiAlgorithm(name, svcLoc),m_dd4hep(nullptr),m_gridDriftChamber(nullptr),
    m_decoder(nullptr)
{
    declareProperty("MCParticle", m_mcParticleCol,
            "Handle of the input MCParticle collection");
    declareProperty("DriftChamberHitsCollection", m_DCSimTrackerHitCol,
            "Handle of DC SimTrackerHit collection");
    declareProperty("DigiDCHitCollection", m_DCDigiCol,
            "Handle of DC digi(TrackerHit) collection");
    declareProperty("DCHitAssociationCollection", m_DCHitAssociationCol,
            "Handle of association collection");
    declareProperty("DCTrackCollection", m_DCTrackCol,
            "Handle of DC track collection");
    declareProperty("SiSubsetTrackCollection", m_siSubsetTrackCol,
            "Handle of silicon subset track collection");
    declareProperty("SDTTrackCollection", m_SDTTrackCol,
            "Handle of SDT track collection");
    declareProperty("VXDTrackerHits", m_VXDTrackerHits,
            "Handle of input VXD tracker hit collection");
    declareProperty("SITTrackerHits", m_SITTrackerHits,
            "Handle of input SIT tracker hit collection");
    declareProperty("SETTrackerHits", m_SETTrackerHits,
            "Handle of input SET tracker hit collection");
    declareProperty("FTDTrackerHits", m_FTDTrackerHits,
            "Handle of input FTD tracker hit collection");
    declareProperty("SITSpacePoints", m_SITSpacePointCol,
            "Handle of input SIT hit collection");
    declareProperty("SETSpacePoints", m_SETSpacePointCol,
            "Handle of input SET hit collection");
    declareProperty("FTDSpacePoints", m_FTDSpacePointCol,
            "Handle of input FTD hit collection");
    declareProperty("VXDCollection", m_VXDCollection,
            "Handle of input VXD hit collection");
    declareProperty("SITCollection", m_SITCollection,
            "Handle of input SIT hit collection");
    declareProperty("SETCollection", m_SETCollection,
            "Handle of input SET hit collection");
    declareProperty("FTDCollection", m_FTDCollection,
            "Handle of input FTD hit collection");
}


StatusCode TruthTrackerAlg::initialize()
{
    ///Get geometry
    m_geomSvc=service<IGeomSvc>("GeomSvc");
    if (!m_geomSvc) {
        error()<<"Failed to get GeomSvc."<<endmsg;
        return StatusCode::FAILURE;
    }
    ///Get Detector
    m_dd4hep=m_geomSvc->lcdd();
    if (nullptr==m_dd4hep) {
        error()<<"Failed to get dd4hep::Detector."<<endmsg;
        return StatusCode::FAILURE;
    }
    //Get Field
    m_dd4hepField=m_geomSvc->lcdd()->field();
    ///Get Readout
    dd4hep::Readout readout=m_dd4hep->readout(m_readout_name);
    ///Get Segmentation
    m_gridDriftChamber=dynamic_cast<dd4hep::DDSegmentation::GridDriftChamber*>
        (readout.segmentation().segmentation());
    if(nullptr==m_gridDriftChamber){
        error()<<"Failed to get the GridDriftChamber"<<endmsg;
        return StatusCode::FAILURE;
    }
    ///Get Decoder
    m_decoder = m_geomSvc->getDecoder(m_readout_name);
    if (nullptr==m_decoder) {
        error()<<"Failed to get the decoder"<<endmsg;
        return StatusCode::FAILURE;
    }

    m_tuple=nullptr;
    ///book tuple
    NTuplePtr nt(ntupleSvc(), "TruthTrackerAlg/truthTrackerAlg");
    if(nt){
        m_tuple=nt;
    }else{
        m_tuple=ntupleSvc()->book("TruthTrackerAlg/truthTrackerAlg",
                CLID_ColumnWiseTuple,"TruthTrackerAlg");
        if(m_tuple){
            StatusCode sc;
            sc=m_tuple->addItem("run",m_run);
            sc=m_tuple->addItem("evt",m_evt);
            sc=m_tuple->addItem("siMom",3,m_siMom);
            sc=m_tuple->addItem("siPos",3,m_siPos);
            sc=m_tuple->addItem("mcMom",3,m_mcMom);
            sc=m_tuple->addItem("mcPos",3,m_mcPos);
            sc=m_tuple->addItem("nSimTrackerHitVXD",m_nSimTrackerHitVXD);
            sc=m_tuple->addItem("nSimTrackerHitSIT",m_nSimTrackerHitSIT);
            sc=m_tuple->addItem("nSimTrackerHitSET",m_nSimTrackerHitSET);
            sc=m_tuple->addItem("nSimTrackerHitFTD",m_nSimTrackerHitFTD);
            sc=m_tuple->addItem("nSimTrackerHitDC",m_nSimTrackerHitDC);
            sc=m_tuple->addItem("nTrackerHitVXD",m_nTrackerHitVXD);
            sc=m_tuple->addItem("nTrackerHitSIT",m_nTrackerHitSIT);
            sc=m_tuple->addItem("nTrackerHitSET",m_nTrackerHitSET);
            sc=m_tuple->addItem("nTrackerHitFTD",m_nTrackerHitFTD);
            sc=m_tuple->addItem("nTrackerHitDC",m_nTrackerHitDC);
            sc=m_tuple->addItem("nSpacePointSIT",m_nSpacePointSIT);
            sc=m_tuple->addItem("nSpacePointSET",m_nSpacePointSET);
            sc=m_tuple->addItem("nSpacePointFTD",m_nSpacePointFTD);
            sc=m_tuple->addItem("nHitOnSiTkXVD",m_nHitOnSiTkVXD);
            sc=m_tuple->addItem("nHitOnSiTkSIT",m_nHitOnSiTkSIT);
            sc=m_tuple->addItem("nHitOnSiTkSET",m_nHitOnSiTkSET);
            sc=m_tuple->addItem("nHitOnSiTkFTD",m_nHitOnSiTkFTD);
            sc=m_tuple->addItem("nHitOnSdtTkVXD",m_nHitOnSdtTkVXD);
            sc=m_tuple->addItem("nHitOnSdtTkSIT",m_nHitOnSdtTkSIT);
            sc=m_tuple->addItem("nHitOnSdtTkSET",m_nHitOnSdtTkSET);
            sc=m_tuple->addItem("nHitOnSdtTkFTD",m_nHitOnSdtTkFTD);
            sc=m_tuple->addItem("nHitOnSdtTkDC",m_nHitOnSdtTkDC);
            sc=m_tuple->addItem("nHitSdt",m_nHitOnSdtTk);
        }
    }
    return GaudiAlgorithm::initialize();
}

StatusCode TruthTrackerAlg::execute()
{
    info()<<"In execute()"<<endmsg;

    ///Output DC Track collection
    edm4hep::TrackCollection* dcTrackCol=m_DCTrackCol.createAndPut();

    ///Output SDT Track collection
    edm4hep::TrackCollection* sdtTrackCol=nullptr;
    if(m_useSi)sdtTrackCol=m_SDTTrackCol.createAndPut();

    ///Retrieve MC particle(s)
    const edm4hep::MCParticleCollection* mcParticleCol=nullptr;
    mcParticleCol=m_mcParticleCol.get();
    //if(m_mcParticleCol.exist()){mcParticleCol=m_mcParticleCol.get();}
    if(nullptr==mcParticleCol){
        debug()<<"MCParticleCollection not found"<<endmsg;
        return StatusCode::SUCCESS;
    }

    ///Retrieve DC digi
    const edm4hep::TrackerHitCollection* digiDCHitsCol=nullptr;
    if(m_useDC){
        digiDCHitsCol=m_DCDigiCol.get();
        if(nullptr==digiDCHitsCol){
            debug()<<"TrackerHitCollection not found"<<endmsg;
        }else{
            debug()<<"digiDCHitsCol size "<<digiDCHitsCol->size()<<endmsg;
            if((int) digiDCHitsCol->size()>m_maxDCDigiCut){
                debug()<<"Track cut by m_maxDCDigiCut "<<m_maxDCDigiCut<<endmsg;
                return StatusCode::SUCCESS;
            }
        }
    }

    ////TODO
    //Output MCRecoTrackerAssociationCollection collection
    //const edm4hep::MCRecoTrackerAssociationCollection*
    //    mcRecoTrackerAssociationCol=nullptr;
    //if(nullptr==mcRecoTrackerAssociationCol){
    //    log<<MSG::DEBUG<<"MCRecoTrackerAssociationCollection not found"
    //        <<endmsg;
    //    return StatusCode::SUCCESS;
    //}
    //mcRecoTrackerAssociationCol=m_mcRecoParticleAssociation.get();

    ///New SDT track
    edm4hep::Track sdtTrack=sdtTrackCol->create();

    int nVXDHit=0;
    int nSITHit=0;
    int nSETHit=0;
    int nFTDHit=0;
    int nDCHitDCTrack=0;
    int nDCHitSDTTrack=0;

    ///Create track with mcParticle
    edm4hep::TrackState trackStateMc;
    getTrackStateFromMcParticle(mcParticleCol,trackStateMc);
    if(m_useTruthTrack.value()){ sdtTrack.addToTrackStates(trackStateMc); }

    if(m_useSi){
        ///Retrieve silicon Track
        const edm4hep::TrackCollection* siTrackCol=nullptr;
        //if(m_siSubsetTrackCol.exist()){
        siTrackCol=m_siSubsetTrackCol.get();
        if(nullptr==siTrackCol){
            debug()<<"SDTTrackCollection is empty"<<endmsg;
            if(!m_useTruthTrack.value()) return StatusCode::SUCCESS;
        }else{
            debug()<<"SiSubsetTrackCol size "<<siTrackCol->size()<<endmsg;
            if(!m_useTruthTrack.value()&&0==siTrackCol->size()){
                return StatusCode::SUCCESS;
            }
        }
        //}

        for(auto siTrack:*siTrackCol){
            if(!m_useTruthTrack.value()){
                debug()<<"siTrack: "<<siTrack<<endmsg;
                edm4hep::TrackState siTrackStat=siTrack.getTrackStates(0);//FIXME?
                sdtTrack.addToTrackStates(siTrackStat);
                sdtTrack.setType(siTrack.getType());
                sdtTrack.setChi2(siTrack.getChi2());
                sdtTrack.setNdf(siTrack.getNdf());
                sdtTrack.setDEdx(siTrack.getDEdx());
                sdtTrack.setDEdxError(siTrack.getDEdxError());
                sdtTrack.setRadiusOfInnermostHit(
                        siTrack.getRadiusOfInnermostHit());
            }
            if(!m_useSiTruthHit){
                debug()<<"use Si hit on track"<<endmsg;
                nVXDHit=addHotsToTrack(
                        siTrack,sdtTrack,lcio::ILDDetID::VXD,"VXD",nVXDHit);
                nSITHit=addHotsToTrack(
                        siTrack,sdtTrack,lcio::ILDDetID::SIT,"SIT",nSITHit);
                nSETHit=addHotsToTrack(
                        siTrack,sdtTrack,lcio::ILDDetID::SET,"SET",nSETHit);
                nFTDHit=addHotsToTrack(
                        siTrack,sdtTrack,lcio::ILDDetID::FTD,"FTD",nFTDHit);
            }//end of loop over hits on siTrack
        }//end of loop over siTrack

        nVXDHit=addHitsToTrack(m_VXDTrackerHits,sdtTrack,"VXD digi",nVXDHit);
        if(m_useSiSpacePoint.value()){
            ///Add vxd silicon trackerHit
            debug()<<"use Si SpacePoint Hit"<<endmsg;
            nSITHit=addHitsToTrack(
                    m_SITSpacePointCol,sdtTrack,"SIT spacePoint",nSITHit);
            nSETHit=addHitsToTrack(
                    m_SETSpacePointCol,sdtTrack,"SET spacePoint",nSETHit);
            nFTDHit=addHitsToTrack(
                    m_FTDSpacePointCol,sdtTrack,"FTD spacePoint",nFTDHit);
        }//end of use space point

        ///Add silicon trackerHit
        debug()<<"use Si TrackerHit"<<endmsg;
        nSITHit=addHitsToTrack(m_SITTrackerHits,sdtTrack,"SIT digi",nSITHit);
        nSETHit=addHitsToTrack(m_SETTrackerHits,sdtTrack,"SET digi",nSETHit);
        nFTDHit=addHitsToTrack(m_FTDTrackerHits,sdtTrack,"FTD digi",nFTDHit);
    }//end of use silicon

    if(m_useDC){
        ///Create DC Track
        edm4hep::Track dcTrack=dcTrackCol->create();
        ///Add DC hits to tracks
        nDCHitDCTrack=addHitsToTrack(
                m_DCDigiCol,dcTrack,"DC digi",nDCHitDCTrack);
        nDCHitSDTTrack=addHitsToTrack(
                m_DCDigiCol,sdtTrack,"DC digi",nDCHitSDTTrack);

        ///Add other track properties
        dcTrack.addToTrackStates(trackStateMc);
        dcTrack.setNdf(dcTrack.trackerHits_size()-5);
        //track.setType();//TODO
        //track.setChi2(gauss(digiDCHitsCol->size-5(),1));//FIXME
        //track.setDEdx();//TODO

        debug()<<"dcTrack nHit "<<dcTrack.trackerHits_size()<<dcTrack<<endmsg;
    }

    ///Set other track parameters
    sdtTrack.setNdf(sdtTrack.trackerHits_size()-5);
    //double radiusOfInnermostHit=1e9;
    //edm4hep::Vector3d digiPos=digiDC.getPosition();
    //double r=sqrt(digiPos.x*digiPos.x+digiPos.y*digiPos.y);
    //if(r<radiusOfInnermostHit) radiusOfInnermostHit=r;

    debug()<<"sdtTrack nHit "<<sdtTrack.trackerHits_size()<<sdtTrack<<endmsg;
    debug()<<"nVXDHit "<<nVXDHit<<" nSITHit "<<nSITHit<<" nSETHit "<<nSETHit
        <<" nFTDHit "<<nFTDHit<<" nDCHitSDTTrack "<<nDCHitSDTTrack<<endmsg;

    if(m_tuple){
        m_nHitOnSdtTkVXD=nVXDHit;
        m_nHitOnSdtTkSIT=nSITHit;
        m_nHitOnSdtTkSET=nSETHit;
        m_nHitOnSdtTkFTD=nFTDHit;
        m_nHitOnSdtTkDC=nDCHitSDTTrack;
        m_nHitOnSdtTk=sdtTrack.trackerHits_size();
        debugEvent();
        StatusCode sc=m_tuple->write();
    }

    return StatusCode::SUCCESS;
}

StatusCode TruthTrackerAlg::finalize()
{
    return GaudiAlgorithm::finalize();
}

void TruthTrackerAlg::getTrackStateFromMcParticle(
        const edm4hep::MCParticleCollection* mcParticleCol,
        edm4hep::TrackState& trackState){
    ///Convert MCParticle to DC Track and ReconstructedParticle
    debug()<<"MCParticleCol size="<<mcParticleCol->size()<<endmsg;
    for(auto mcParticle : *mcParticleCol){
        /// skip mcParticleVertex do not have enough associated hits TODO
        ///Vertex
        const edm4hep::Vector3d mcParticleVertex=mcParticle.getVertex();//mm
        edm4hep::Vector3f mcParticleVertexSmeared;//mm
        mcParticleVertexSmeared.x=
            CLHEP::RandGauss::shoot(mcParticleVertex.x,m_resVertexX);
        mcParticleVertexSmeared.y=
            CLHEP::RandGauss::shoot(mcParticleVertex.y,m_resVertexY);
        mcParticleVertexSmeared.z=
            CLHEP::RandGauss::shoot(mcParticleVertex.z,m_resVertexZ);
        ///Momentum
        edm4hep::Vector3f mcParticleMom=mcParticle.getMomentum();//GeV
        double mcParticlePt=sqrt(mcParticleMom.x*mcParticleMom.x+
                mcParticleMom.y*mcParticleMom.y);
        //double mcParticlePtSmeared=
        //    CLHEP::RandGauss::shoot(mcParticlePt,m_resPT);
        double mcParticleMomPhi=atan2(mcParticleMom.y,mcParticleMom.x);
        double mcParticleMomPhiSmeared=
            CLHEP::RandGauss::shoot(mcParticleMomPhi,m_resMomPhi);
        edm4hep::Vector3f mcParticleMomSmeared;
        mcParticleMomSmeared.x=mcParticlePt*cos(mcParticleMomPhiSmeared);
        mcParticleMomSmeared.y=mcParticlePt*sin(mcParticleMomPhiSmeared);
        mcParticleMomSmeared.z=
            CLHEP::RandGauss::shoot(mcParticleMom.z,m_resPz);

        ///Converted to Helix
        double B[3]={1e9,1e9,1e9};
        m_dd4hepField.magneticField({0.,0.,0.},B);
        HelixClass helix;
        //float pos[3]={mcParticleVertexSmeared.x,
        //    mcParticleVertexSmeared.y,mcParticleVertexSmeared.z};
        //float mom[3]={mcParticleMomSmeared.x,mcParticleMomSmeared.y,
        //    mcParticleMomSmeared.z};
        ////FIXME DEBUG
        float pos[3]={(float)mcParticleVertex.x,
            (float)mcParticleVertex.y,(float)mcParticleVertex.z};
        float mom[3]={(float)mcParticleMom.x,(float)mcParticleMom.y,
            (float)mcParticleMom.z};
        helix.Initialize_VP(pos,mom,mcParticle.getCharge(),B[2]/dd4hep::tesla);
        if(m_tuple) {
            for(int ii=0;ii<3;ii++) {
                m_mcMom[ii]=mom[ii];
                m_mcPos[ii]=pos[ii];
            }
        }

        ///new Track
        trackState.D0=helix.getD0();
        trackState.phi=helix.getPhi0();
        trackState.omega=helix.getOmega();
        trackState.Z0=helix.getZ0();
        trackState.tanLambda=helix.getTanLambda();
        trackState.referencePoint=helix.getReferencePoint();
        std::array<float,15> covMatrix;
        for(int i=0;i<15;i++){covMatrix[i]=100.;}//FIXME
        trackState.covMatrix=covMatrix;

        debug()<<"mcParticle "<<mcParticle
            <<" momPhi "<<mcParticleMomPhi
            <<" mcParticleVertex("<<mcParticleVertex<<")mm "
            <<" mcParticleVertexSmeared("<<mcParticleVertexSmeared<<")mm "
            <<" mcParticleMom("<<mcParticleMom<<")GeV "
            <<" mcParticleMomSmeared("<<mcParticleMomSmeared<<")GeV "
            <<" Bxyz "<<B[0]/dd4hep::tesla<<" "<<B[1]/dd4hep::tesla
            <<" "<<B[2]/dd4hep::tesla<<" tesla"<<endmsg;
    }//end loop over MCParticleCol
}//end of getTrackStateFromMcParticle

void TruthTrackerAlg::debugEvent()
{
    ///Retrieve silicon Track
    const edm4hep::TrackCollection* siTrackCol=nullptr;
    siTrackCol=m_siSubsetTrackCol.get();
    if(nullptr!=siTrackCol){
        for(auto siTrack:*siTrackCol){
            debug()<<"siTrack: "<<siTrack<<endmsg;
            edm4hep::TrackState trackStat=siTrack.getTrackStates(0);//FIXME?
            double B[3]={1e9,1e9,1e9};
            m_dd4hepField.magneticField({0.,0.,0.},B);
            HelixClass helix;
            helix.Initialize_Canonical(trackStat.phi, trackStat.D0, trackStat.Z0,
                    trackStat.omega, trackStat.tanLambda, B[2]/dd4hep::tesla);

            m_siMom[0]=helix.getMomentum()[0];
            m_siMom[1]=helix.getMomentum()[1];
            m_siMom[2]=helix.getMomentum()[2];
            m_siPos[0]=helix.getReferencePoint()[0];
            m_siPos[1]=helix.getReferencePoint()[1];
            m_siPos[2]=helix.getReferencePoint()[2];
            m_nHitOnSiTkVXD=nHotsOnTrack(siTrack,lcio::ILDDetID::VXD);
            m_nHitOnSiTkSIT=nHotsOnTrack(siTrack,lcio::ILDDetID::SIT);
            m_nHitOnSiTkSET=nHotsOnTrack(siTrack,lcio::ILDDetID::SET);
            m_nHitOnSiTkFTD=nHotsOnTrack(siTrack,lcio::ILDDetID::FTD);
        }//end of loop over siTrack
    }
    //SimTrackerHits
    m_nSimTrackerHitVXD=simTrackerHitColSize(m_VXDCollection);
    m_nSimTrackerHitSIT=simTrackerHitColSize(m_SITCollection);
    m_nSimTrackerHitSET=simTrackerHitColSize(m_SETCollection);
    m_nSimTrackerHitFTD=simTrackerHitColSize(m_FTDCollection);

    //TrackerHits
    m_nTrackerHitVXD=trackerHitColSize(m_VXDTrackerHits);
    m_nTrackerHitSIT=trackerHitColSize(m_SITTrackerHits);
    m_nTrackerHitSET=trackerHitColSize(m_SETTrackerHits);
    m_nTrackerHitFTD=trackerHitColSize(m_FTDTrackerHits);
    m_nTrackerHitDC=trackerHitColSize(m_DCDigiCol);

    //SpacePoints
    m_nSpacePointSIT=trackerHitColSize(m_SITSpacePointCol);
    m_nSpacePointSET=trackerHitColSize(m_SETSpacePointCol);
    m_nSpacePointFTD=trackerHitColSize(m_FTDSpacePointCol);
}

int TruthTrackerAlg::addHitsToTrack(DataHandle<edm4hep::TrackerHitCollection>&
        colHandle, edm4hep::Track& track, const char* msg,int nHitAdded)
{
    if(nHitAdded>0) return nHitAdded;
    int nHit=0;
    const edm4hep::TrackerHitCollection* col=colHandle.get();
    debug()<<"add "<<msg<<" "<<col->size()<<" trackerHit"<<endmsg;
    for(auto hit:*col){
        track.addToTrackerHits(hit);
        ++nHit;
    }
    return nHit;
}

int TruthTrackerAlg::addHotsToTrack(edm4hep::Track& sourceTrack,
        edm4hep::Track& targetTrack, int hitType,const char* msg,int nHitAdded)
{
    if(nHitAdded>0) return nHitAdded;
    int nHit=0;
    for(unsigned int iHit=0;iHit<sourceTrack.trackerHits_size();iHit++){
        edm4hep::ConstTrackerHit hit=sourceTrack.getTrackerHits(iHit);
        UTIL::BitField64 encoder(lcio::ILDCellID0::encoder_string);
        encoder.setValue(hit.getCellID());
        if(encoder[lcio::ILDCellID0::subdet]==hitType){
            targetTrack.addToTrackerHits(hit);
            debug()<<endmsg<<" add siHit "<<iHit<<" "<<hit<<endmsg;//got error
            ++nHit;
        }
    }
    debug()<<endmsg<<" add "<<nHit<<" "<<msg<<" hit on track"<<endmsg;//got error
    return nHit;
}

int TruthTrackerAlg::nHotsOnTrack(edm4hep::Track& track, int hitType)
{
    int nHit=0;
    for(unsigned int iHit=0;iHit<track.trackerHits_size();iHit++){
        edm4hep::ConstTrackerHit hit=track.getTrackerHits(iHit);
        UTIL::BitField64 encoder(lcio::ILDCellID0::encoder_string);
        encoder.setValue(hit.getCellID());
        if(encoder[lcio::ILDCellID0::subdet]==hitType){
            ++nHit;
        }
    }
    return nHit;
}

int TruthTrackerAlg::trackerHitColSize(DataHandle<edm4hep::TrackerHitCollection>& col)
{
    const edm4hep::TrackerHitCollection* c=col.get();
    if(nullptr!=c) return c->size();
    return 0;
}

int TruthTrackerAlg::simTrackerHitColSize(DataHandle<edm4hep::SimTrackerHitCollection>& col)
{
    const edm4hep::SimTrackerHitCollection* c=col.get();
    if(nullptr!=c) return c->size();
    return 0;
}
