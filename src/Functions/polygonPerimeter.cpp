#include <Functions/FunctionFactory.h>
#include <Functions/geometryConverters.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <common/logger_useful.h>

#include <Columns/ColumnArray.h>
#include <Columns/ColumnTuple.h>
#include <Columns/ColumnsNumber.h>
#include <DataTypes/DataTypeArray.h>
#include <DataTypes/DataTypeTuple.h>
#include <DataTypes/DataTypeCustomGeo.h>

#include <memory>
#include <string>

namespace DB
{

namespace ErrorCodes
{
    extern const int TOO_FEW_ARGUMENTS_FOR_FUNCTION;
    extern const int BAD_ARGUMENTS;
    extern const int ILLEGAL_TYPE_OF_ARGUMENT;
    extern const int ILLEGAL_COLUMN;
}


using CoordinateType = Float64;
using Point = boost::geometry::model::d2::point_xy<CoordinateType>;
using Polygon = boost::geometry::model::polygon<Point, false>;
using MultiPolygon = boost::geometry::model::multi_polygon<Float64Polygon>;
using Box = boost::geometry::model::box<Point>;


class FunctionPolygonPerimeter : public IFunction
{
public:
    static inline const char * name = "polygonPerimeter";

    explicit FunctionPolygonPerimeter() = default;

    static FunctionPtr create(const Context &)
    {
        return std::make_shared<FunctionPolygonPerimeter>();
    }

    String getName() const override
    {
        return name;
    }

    bool isVariadic() const override
    {
        return false;
    }

    size_t getNumberOfArguments() const override
    {
        return 1;
    }

    DataTypePtr getReturnTypeImpl(const DataTypes &) const override
    {
        return std::make_shared<DataTypeFloat64>();
    }

    void executeImpl(Block & block, const ColumnNumbers & arguments, size_t result, size_t input_rows_count) override
    {
        auto get_parser = [&block, &arguments] (size_t i) {
            const ColumnWithTypeAndName polygon = block.getByPosition(arguments[i]);
            return makeGeometryFromColumnParser(polygon);
        };

        auto parser = get_parser(0);
        auto container = createContainer(parser);

        auto res_column = ColumnFloat64::create();

        for (size_t i = 0; i < input_rows_count; i++)
        {
            get(parser, container, i);

            Float64 perimeter = boost::geometry::perimeter(
                boost::get<Float64MultiPolygon>(container));

            res_column->insertValue(perimeter);
        }

        block.getByPosition(result).column = std::move(res_column);
    }

    bool useDefaultImplementationForConstants() const override
    {
        return true;
    }
};


void registerFunctionPolygonPerimeter(FunctionFactory & factory)
{
    factory.registerFunction<FunctionPolygonPerimeter>();
}

}
