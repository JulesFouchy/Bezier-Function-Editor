#include <p6/p6.h>

static constexpr float pt_radius = 0.05f;

struct Bezier {
    std::array<glm::vec2, 4> pts;

    auto A() const { return pts[0].x; }
    auto B() const { return pts[1].x; }
    auto C() const { return pts[2].x; }
    auto D() const { return pts[3].x; }

    auto eval(float t) const -> glm::vec2
    {
        return pts[0] * (1.f - t) * (1.f - t) * (1.f - t) +
               pts[1] * 3.f * t * (1.f - t) * (1.f - t) +
               pts[2] * 3.f * t * t * (1.f - t) +
               pts[3] * t * t * t;
    }

    void draw(p6::Context& ctx) const
    {
        { // Points
            for (auto const& pt : pts)
                ctx.circle(p6::Center{pt}, p6::Radius{pt_radius});

            ctx.stroke.a() = 0.1f;
            ctx.line(pts[0], pts[1]);
            ctx.line(pts[3], pts[2]);
            ctx.stroke.a() = 1.f;
        }

        { // Actual curve
            glm::vec2 prev = pts[0];
            for (int i = 1; i <= 150; ++i)
            {
                float t = i / 150.f;

                const float a = -3.f * A() + 9.f * B() - 9.f * C() + 3.f * D();
                const float b = 6.f * A() - 12.f * B() + 6.f * C();
                const float c = -3.f * A() + 3.f * B();

                const float derivative = a * t * t + b * t + c;
                if (derivative >= 0)
                    ctx.stroke = p6::NamedColor::GreenPigment;
                else
                    ctx.stroke = p6::NamedColor::RedViolet;

                glm::vec2 curr = eval(t);
                ctx.line(prev, curr);
                prev = curr;
            }
        }
    }

    auto is_valid_function() const -> bool
    {
        // We check that Bezier(t).x is an increasing function
        // It's derivative is of the form a*t^2 + b*t + c
        // and we check that it is always > 0
        const float a = -3.f * A() + 9.f * B() - 9.f * C() + 3.f * D();
        const float b = 6.f * A() - 12.f * B() + 6.f * C();
        const float c = -3.f * A() + 3.f * B();

        // TODO handle when a == 0.f and we have a straight line
        if (a == 0.f)
        {
            return false;
        }

        const float delta = b * b - 4.f * a * c;
        if (delta < 0.f)
        {
            ImGui::Text("Delta Négatif");
            return a > 0.f;
        }
        else if (delta > 0.f)
        {
            ImGui::Text("Delta Positif");
            const float sign  = a > 0.f ? 1.f : -1.f;
            const float root1 = (-b - sign * std::sqrt(delta)) / (2.f * a);
            const float root2 = (-b + sign * std::sqrt(delta)) / (2.f * a);

            ImGui::Text("Root1: %.3f", root1);
            ImGui::Text("Root2: %.3f", root2);

            if (a > 0.f)
            {
                ImGui::Text("a Positif");
                return root2 <= 0.f ||
                       root1 >= 1.f;
            }
            else
            {
                ImGui::Text("a Négatif");
                return root1 <= 0.f && root2 >= 1.f;
            }
        }
        else // delta == 0
        {
            return true; // TODO
        }
    }
};

auto main() -> int
{
    auto curve = Bezier{{
        glm::vec2{-0.5, -0.5},
        glm::vec2{-0.2, 0.5},
        glm::vec2{0.2, 0.5},
        glm::vec2{0.5, -0.5},
    }};

    auto ctx = p6::Context{{1280, 720, "Bezier Function Editor"}};
    ctx.maximize_window();

    ctx.update = [&]() {
        ImGui::Begin("Test");
        ctx.background(
            curve.is_valid_function()
                ? p6::NamedColor::DeepSkyBlue
                : p6::NamedColor::RedDevil
        );
        curve.draw(ctx);
        ImGui::End();
    };

    std::optional<size_t> selected{};

    ctx.mouse_pressed = [&](auto&&) {
        size_t i = 0;
        for (auto const& pt : curve.pts)
        {
            if (glm::distance(pt, ctx.mouse()) < pt_radius)
                selected = i;
            i++;
        }
    };

    ctx.mouse_released = [&](auto&&) {
        selected = std::nullopt;
    };

    ctx.mouse_dragged = [&](p6::MouseDrag e) {
        if (!selected)
            return;
        curve.pts[*selected] += e.delta;

        // Also move the control points associated with the anchor points
        if (*selected == 0)
            curve.pts[1] += e.delta;
        if (*selected == 3)
            curve.pts[2] += e.delta;
    };

    ctx.start();
}