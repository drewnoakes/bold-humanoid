%{
#include <VisualCortex/visualcortex.hh>
%}


namespace bold
{
  /** Bold-humanoid's vision processing subsystem. */
  class VisualCortex
  {
  public:
    void setShouldDetectLines(bool val) { d_shouldDetectLines = val; }
    bool getShouldDetectLines() const { return d_shouldDetectLines; }

    void setShouldIgnoreAboveHorizon(bool val) { d_shouldIgnoreAboveHorizon = val; }
    bool getShouldIgnoreAboveHorizon() const { return d_shouldIgnoreAboveHorizon; }

    void setMinBallArea(unsigned val) { d_minBallArea = val; }
    unsigned getMinBallArea() const { return d_minBallArea; }

    void setStreamFramePeriod(unsigned val) { d_streamFramePeriod = val; }
    unsigned getStreamFramePeriod() const { return d_streamFramePeriod; }
    
    void setShouldDrawBlobs(bool val) { d_shouldDrawBlobs = val; }
    bool getShouldDrawBlobs() const { return d_shouldDrawBlobs; }

    void setShouldDrawLineDots(bool val) { d_shouldDrawLineDots = val; }
    bool getShouldDrawLineDots() const { return d_shouldDrawLineDots; }

    void setShouldDrawExpectedLines(bool val) { d_shouldDrawExpectedLines = val; }
    bool getShouldDrawExpectedLines() const { return d_shouldDrawExpectedLines; }

    void setShouldDrawObservedLines(bool val) { d_shouldDrawObservedLines = val; }
    bool getShouldDrawObservedLines() const { return d_shouldDrawObservedLines; }

    void setShouldDrawHorizon(bool val) { d_shouldDrawHorizon = val; }
    bool getShouldDrawHorizon() const { return d_shouldDrawHorizon; }
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
