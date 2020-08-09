

class MouseInput {

    constructor() {
        this.isActive = false;
        this.prev = {x: 0.0, y: 0.0};
        this.curr = {x: 0.0, y: 0.0};
        this.dxy = {x: 0.0, y: 0.0}
        this.handleFunction = () => {};
    }

    setMouseMoveCallback(handleFunction) {
        this.handleFunction = handleFunction;
    }

    activate() {
        this.isActive = true;
    }

    deactivate() {
        this.isActive = false;
    }

    getX() {
        return this.curr.x;
    }

    getY() {
        return this.curr.y;
    }

    getDeltaX() {
        return this.dxy.x;
    }

    getDeltaY() {
        return this.dxy.y;
    }

    handleMouseInput(event, mouseMove=true) {
        let x = event.clientX;
        let y = event.clientY;
        if (mouseMove && this.isActive) {
            this.dxy.x = x - this.prev.x
            this.dxy.y = y - this.prev.y
            this.prev.x = this.curr.x;
            this.prev.y = this.curr.y;
            this.curr.x = x;
            this.curr.y = y;
            this.handleFunction(this);
        } else if (!this.isActive) {
            this.curr.x = x;
            this.curr.y = y;
            this.prev.x = x;
            this.prev.y = y;
            if (!mouseMove) this.isActive = true;
        }
    }
}


export {MouseInput}