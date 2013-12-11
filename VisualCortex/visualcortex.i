%{
#include <VisualCortex/visualcortex.hh>
%}


namespace bold
{
  /** Bold-humanoid's vision processing subsystem. */
  class VisualCortex
  {
  public:
    void setShouldDetectLines(bool val) { d_shouldDetectLines->setValue(val); }
    bool getShouldDetectLines() const { return d_shouldDetectLines->getValue(); }

    void setShouldIgnoreAboveHorizon(bool val) { d_shouldIgnoreAboveHorizon->setValue(val); }
    bool getShouldIgnoreAboveHorizon() const { return d_shouldIgnoreAboveHorizon->getValue(); }

    void setMinBallArea(unsigned val) { d_minBallArea->setValue(val); }
    unsigned getMinBallArea() const { return d_minBallArea->getValue(); }

    void setStreamFramePeriod(unsigned val) { d_streamFramePeriod->setValue(val); }
    unsigned getStreamFramePeriod() const { return d_streamFramePeriod->getValue(); }

    void setShouldDrawBlobs(bool val) { d_shouldDrawBlobs->setValue(val); }
    bool getShouldDrawBlobs() const { return d_shouldDrawBlobs->getValue(); }

    void setShouldDrawLineDots(bool val) { d_shouldDrawLineDots->setValue(val); }
    bool getShouldDrawLineDots() const { return d_shouldDrawLineDots->getValue(); }

    void setShouldDrawExpectedLines(bool val) { d_shouldDrawExpectedLines->setValue(val); }
    bool getShouldDrawExpectedLines() const { return d_shouldDrawExpectedLines->getValue(); }

    void setShouldDrawExpectedLineEdges(bool val) { d_shouldDrawExpectedLineEdges->setValue(val); }
    bool getShouldDrawExpectedLineEdges() const { return d_shouldDrawExpectedLineEdges->getValue(); }

    void setShouldDrawObservedLines(bool val) { d_shouldDrawObservedLines->setValue(val); }
    bool getShouldDrawObservedLines() const { return d_shouldDrawObservedLines->getValue(); }

    void setShouldDrawHorizon(bool val) { d_shouldDrawHorizon->setValue(val); }
    bool getShouldDrawHorizon() const { return d_shouldDrawHorizon->getValue(); }
  };

  %extend VisualCortex {
  public:
    void set(bool shouldDetectLines, bool shouldIgnoreAboveHorizon,
             unsigned minBallArea, unsigned streamFramePeriod,
             bool shouldDrawBlobs, bool shouldDrawLineDots, bool shouldDrawExpectedLines, bool shouldDrawObservedLines, bool shouldDrawHorizon)
    {
      $self->setShouldDetectLines(shouldDetectLines);
      $self->setShouldIgnoreAboveHorizon(shouldIgnoreAboveHorizon);
      $self->setMinBallArea(minBallArea);
      $self->setStreamFramePeriod(streamFramePeriod);
      $self->setShouldDrawBlobs(shouldDrawBlobs);
      $self->setShouldDrawLineDots(shouldDrawLineDots);
      $self->setShouldDrawExpectedLines(shouldDrawExpectedLines);
      $self->setShouldDrawObservedLines(shouldDrawObservedLines);
      $self->setShouldDrawHorizon(shouldDrawHorizon);
    }
  };
}
