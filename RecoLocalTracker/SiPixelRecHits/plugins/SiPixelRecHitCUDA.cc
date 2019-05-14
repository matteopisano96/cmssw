#include <cuda_runtime.h>

#include "CUDADataFormats/BeamSpot/interface/BeamSpotCUDA.h"
#include "CUDADataFormats/Common/interface/CUDAProduct.h"
#include "CUDADataFormats/Common/interface/CUDAProduct.h"
#include "CUDADataFormats/SiPixelCluster/interface/SiPixelClustersCUDA.h"
#include "CUDADataFormats/SiPixelDigi/interface/SiPixelDigisCUDA.h"
#include "CUDADataFormats/TrackingRecHit/interface/TrackingRecHit2DCUDA.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "HeterogeneousCore/CUDACore/interface/CUDAScopedContext.h"
#include "RecoLocalTracker/Records/interface/TkPixelCPERecord.h"
#include "RecoLocalTracker/SiPixelRecHits/interface/PixelCPEBase.h"
#include "RecoLocalTracker/SiPixelRecHits/interface/PixelCPEFast.h"

#include "PixelRecHits.h"  // TODO : spit product from kernel

class SiPixelRecHitCUDA : public edm::global::EDProducer<> {
public:
  explicit SiPixelRecHitCUDA(const edm::ParameterSet& iConfig);
  ~SiPixelRecHitCUDA() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::StreamID streamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;

  // The mess with inputs will be cleaned up when migrating to the new framework
  edm::EDGetTokenT<CUDAProduct<BeamSpotCUDA>> tBeamSpot;
  edm::EDGetTokenT<CUDAProduct<SiPixelClustersCUDA>> token_;
  edm::EDGetTokenT<CUDAProduct<SiPixelDigisCUDA>> tokenDigi_;

  edm::EDPutTokenT<CUDAProduct<TrackingRecHit2DCUDA>> tokenHit_;

  std::string cpeName_;

  pixelgpudetails::PixelRecHitGPUKernel gpuAlgo_;
};

SiPixelRecHitCUDA::SiPixelRecHitCUDA(const edm::ParameterSet& iConfig)
    : tBeamSpot(consumes<CUDAProduct<BeamSpotCUDA>>(iConfig.getParameter<edm::InputTag>("beamSpot"))),
      token_(consumes<CUDAProduct<SiPixelClustersCUDA>>(iConfig.getParameter<edm::InputTag>("src"))),
      tokenDigi_(consumes<CUDAProduct<SiPixelDigisCUDA>>(iConfig.getParameter<edm::InputTag>("src"))),
      tokenHit_(produces<CUDAProduct<TrackingRecHit2DCUDA>>()),
      cpeName_(iConfig.getParameter<std::string>("CPE")) {}

void SiPixelRecHitCUDA::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>("beamSpot", edm::InputTag("offlineBeamSpotCUDA"));
  desc.add<edm::InputTag>("src", edm::InputTag("siPixelClustersCUDAPreSplitting"));
  desc.add<std::string>("CPE", "PixelCPEFast");
  descriptions.add("siPixelRecHitCUDA", desc);
}

void SiPixelRecHitCUDA::produce(edm::StreamID streamID, edm::Event& iEvent, const edm::EventSetup& es) const {
  // const TrackerGeometry *geom_ = nullptr;
  const PixelClusterParameterEstimator* cpe_ = nullptr;

  /*
  edm::ESHandle<TrackerGeometry> geom;
  es.get<TrackerDigiGeometryRecord>().get( geom );
  geom_ = geom.product();
  */

  edm::ESHandle<PixelClusterParameterEstimator> hCPE;
  es.get<TkPixelCPERecord>().get(cpeName_, hCPE);
  cpe_ = dynamic_cast<const PixelCPEBase*>(hCPE.product());

  PixelCPEFast const* fcpe = dynamic_cast<const PixelCPEFast*>(cpe_);
  if (!fcpe) {
    throw cms::Exception("Configuration") << "too bad, not a fast cpe gpu processing not possible....";
  }

  edm::Handle<CUDAProduct<SiPixelClustersCUDA>> hclusters;
  iEvent.getByToken(token_, hclusters);

  CUDAScopedContext ctx{*hclusters};
  auto const& clusters = ctx.get(*hclusters);

  edm::Handle<CUDAProduct<SiPixelDigisCUDA>> hdigis;
  iEvent.getByToken(tokenDigi_, hdigis);
  auto const& digis = ctx.get(*hdigis);

  edm::Handle<CUDAProduct<BeamSpotCUDA>> hbs;
  iEvent.getByToken(tBeamSpot, hbs);
  auto const& bs = ctx.get(*hbs);

  ctx.emplace(
      iEvent,
      tokenHit_,
      std::move(gpuAlgo_.makeHitsAsync(digis, clusters, bs, fcpe->getGPUProductAsync(ctx.stream()), ctx.stream())));
}

DEFINE_FWK_MODULE(SiPixelRecHitCUDA);
