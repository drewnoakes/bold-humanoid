/**
 * @author Drew Noakes https://drewnoakes.com
 */

class Animator
{
    private needsRender: boolean;
    private stopAnimation: boolean;

    constructor(private callback: ()=>void)
    {}

    public start(): void
    {
        this.needsRender = true;
        this.stopAnimation = false;

        this.animate();
    }

    public stop(): void
    {
        this.stopAnimation = true;
    }

    public setRenderNeeded(): void
    {
        this.needsRender = true;
    }

    private animate()
    {
        if (this.stopAnimation)
            return;

        window.requestAnimationFrame(this.animate.bind(this));

        if (this.needsRender)
        {
            this.callback();
            this.needsRender = false;
        }
    }
}

export = Animator;
