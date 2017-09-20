#ifndef IOPool_Output_PoolOutputModule_h
#define IOPool_Output_PoolOutputModule_h

//////////////////////////////////////////////////////////////////////
//
// Class PoolOutputModule. Output module to POOL file
//
// Oringinal Author: Luca Lista
// Current Author: Bill Tanenbaum
//
//////////////////////////////////////////////////////////////////////

#include "IOPool/Output/interface/PoolOutputModuleBase.h"
#include "FWCore/Framework/interface/one/OutputModule.h"

class TTree;
namespace edm {

  class PoolOutputModule : public one::OutputModule<WatchInputFiles>, public PoolOutputModuleBase {
  public:
    explicit PoolOutputModule(ParameterSet const& ps);
    virtual ~PoolOutputModule();
    PoolOutputModule(PoolOutputModule const&) = delete; // Disallow copying and moving
    PoolOutputModule& operator=(PoolOutputModule const&) = delete; // Disallow copying and moving

    std::string const& currentFileName() const;

    static void fillDescription(ParameterSetDescription& desc);
    static void fillDescriptions(ConfigurationDescriptions& descriptions);

    using OutputModule::selectorConfig;

    // these must be forwarded by the OutputModule implementation
    virtual bool OMwantAllEvents() const override;
    virtual BranchIDLists const* OMbranchIDLists() override;
    virtual ThinnedAssociationsHelper const* OMthinnedAssociationsHelper() const override;
    virtual ParameterSetID OMselectorConfig() const override;
    virtual SelectedProductsForBranchType const& OMkeptProducts() const override;

  protected:
    ///allow inheriting classes to override but still be able to call this method in the overridden version
    virtual bool shouldWeCloseFile() const override;
    virtual void write(EventForOutput const& e) override;

  private:
    virtual void preActionBeforeRunEventAsync(WaitingTask* iTask, ModuleCallingContext const& iModuleCallingContext, Principal const& iPrincipal) const override;

    virtual void openFile(FileBlock const& fb) override;
    virtual void respondToOpenInputFile(FileBlock const& fb) override;
    virtual void respondToCloseInputFile(FileBlock const& fb) override;
    virtual void writeLuminosityBlock(LuminosityBlockForOutput const& lb) override;
    virtual void writeRun(RunForOutput const& r) override;
    virtual bool isFileOpen() const override;
    void reallyOpenFile();
    virtual void reallyCloseFile() override;
    virtual void beginJob() override;

    void beginInputFile(FileBlock const& fb);

    edm::propagate_const<std::unique_ptr<RootOutputFile>> rootOutputFile_;
  };
}

#endif

